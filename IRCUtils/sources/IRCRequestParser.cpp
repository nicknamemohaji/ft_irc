#include "IRCUtils/includes/IRCRequestParser.hpp"

#include <string>
#include <algorithm>

#include "IRCServer/includes/IRCTypes.hpp"
#include "IRCServer/includes/IRCErrors.hpp"

/*
IRCRequestParser.cpp
- bool IRC_request_parser::ParseMessage
- void IRC_request_parser::AddNewLineToBuffer
- std::string IRC_request_parser::AddChanPrefixToParam
- std::string IRC_request_parser::DelChanPrefixFromParam
- enum IRCCommand IRC_request_parser::ConvertStrToCom
- std::string IRC_request_parser::ConvertComToStr
*/

static std::string  BufferParseUntilSpace(Buffer* message, bool* result);
static void BufferRemoveSpace(Buffer* message);

/**
 * @brief 버퍼에서 공백 또는 개행까지를 자르고, 버퍼에서 제거합니다
 * 
 * @param[in,out] message 자를 메시지
 * @param[out] result 파싱 결과. 개행 또는 공백을 찾을 수 없으면 false로 설정
 * 
 * @return 첫 공백 또는 개행까지의 문자열
*/
static std::string BufferParseUntilSpace(Buffer* message, bool* result) {
  // find
  Buffer::iterator it_SP = std::find(message->begin(), message->end(), ' ');
  Buffer::iterator it_CR = std::find(message->begin(), message->end(), '\r');
  if (it_SP == message->end() && it_CR == message->end()) {
    *result = false;
    return "";
  }

  // compare SP and CR position
  std::string ret;
  unsigned int dist_sp = std::distance(message->begin(), it_SP);
  unsigned int dist_cr = std::distance(message->begin(), it_CR);
  if (dist_sp < dist_cr) {
    // space is first occurance
    ret.assign(message->begin(), it_SP);
    message->erase(message->begin(), it_SP);
  } else {
    // carriage return is first occurance
    ret.assign(message->begin(), it_CR);
    message->erase(message->begin(), it_CR);
  }

  *result = true;
  return ret;
}

/**
 * @brief 버퍼의 첫 위치부터 연속된 공백을 제거합니다
 * 
 * @param[in,out] message 공백을 제거할 메시지
*/
static void BufferRemoveSpace(Buffer* message) {
  // SPACE ::= %x20 *(%x20)
  // so should handle multiple space characters
  while (message->begin() != message->end() && *(message->begin()) == ' ')
    message->erase(message->begin());
}


/**
 * @brief 버퍼를 파싱해 IRC 메시지를(커맨드, 인자) 분리합니다
 * 
 * @param[in,out] message 파싱할 버퍼. 첫 개행까지 파싱하고 버퍼에서 메시지를 제거합니다
 * @param[out] command 파싱된 커맨드
 * @param[out] params 파싱된 인자
 * 
 * @return 오류가 있으면 false를 반환합니다
 * 
 * @note 반드시 IRC_request_parser::AddNewLineToBuffer를 거쳐 CRLF 쌍이 맞춰진
 * 버퍼가 들어와야 합니다
*/
bool IRC_request_parser::ParseMessage(
  Buffer* message, IRCCommand* command, IRCParams* params) {
  // 1. ['@' <tags> SPACE] our server will disable tag.
  // 2. [':' <source> SPACE ] client MUST NOT send source to server
  if ((*message)[0] == ':')
    return false;

  // 3. <command>
  bool _valid_command;
  std::string _raw_command = BufferParseUntilSpace(message, &_valid_command);
  if (!_valid_command)
    return false;
  BufferRemoveSpace(message);
  *command = IRC_request_parser::ConvertStrToCom(_raw_command);

  // 4. <parameters>
  while (*(message->begin()) != '\r') {
    // parameters ::= *( SPACE middle ) [ SPACE ":" trailing ]
    // middle   ::=  nospcrlfcl *( ":" / nospcrlfcl )
    // trailing   ::=  *( ":" / " " / nospcrlfcl )
    Buffer::iterator _it;
    std::string _param;

    // trailing parameter
    if ((*message)[0] == ':') {
      _it = std::find(message->begin(), message->end(), '\r');
      // ignore first `:`
      _param.assign(message->begin() + 1, _it);
      // remove sliced message
      message->erase(message->begin(), _it);
    } else {
      // middle parameter
      _param = BufferParseUntilSpace(message, &_valid_command);
      if (!_valid_command)
        return true;
    }

    params->push_back(_param);
    BufferRemoveSpace(message);
  }

  // 5. <crlf>
  message->erase(message->begin(), message->begin() + 2);

  return true;
}

/**
 * @brief 버퍼에 CR 또는 LF가 하나만 있으면 가장 처음 온 개행 문자의 CRLF 쌍을 맞춥니다
 * 
 * @param[in,out] buffer 쌍을 맞출 버퍼
 * 
 * @note AddNewLineToBuffer는 short count를 고려하지 않습니다.
 * 반드시 버퍼에 CR 또는 LF가 하나라도 있어야 합니다.
 * 
 * @note notes on inserting CR or LF
 * ircv3 표준과 modern irc 문서는 호환성을 위해 단일 LF(\n)만 오는 메시지 처리를 권장하고(SHOULD), 
 * 단일 CR(\r)만 오는 메시지 처리를 제안합니다(MAY). 
 * RFC 1459에서도 호환성과 관련해 단일 LF를 인식하는 서버가 있다고 언급하고 있습니다. (단, 이에 대한 요구사항은 없습니다.)
 * 이 구현은 modern irc 표준을 따르므로, 단일 LF 또는 CR이 올 경우에 CRLF 짝을 맞추도록 추가하였습니다.
*/
void IRC_request_parser::AddNewLineToBuffer(Buffer* buffer) {
  Buffer::iterator it_cr = std::find(buffer->begin(), buffer->end(), '\r');
  if (it_cr == buffer->end()) {
    // if CR is not set, append CR in front of LF to match set
    Buffer::iterator it_lf = std::find(buffer->begin(), buffer->end(), '\n');
    buffer->insert(it_lf, '\r');
  }  else if ((it_cr + 1) == buffer->end() || *(it_cr + 1) != '\n') {
    // if LF is not set, append LF after CR to match set
    buffer->insert(it_cr + 1, '\n');
  }
}

IRCParams IRC_request_parser::SeparateParam(const std::string& param,
                                            const std::string& delim) {
    IRCParams ret;
    size_t start = 0;
    size_t end = param.find(delim);

    while (end != std::string::npos) {
        ret.push_back(param.substr(start, end - start));
        start = end + delim.length();
        end = param.find(delim, start);
    }
    // 마지막 요소 추가
    ret.push_back(param.substr(start));

    return ret;
}

std::string IRC_request_parser::AddChanPrefixToParam(const std::string& param) {
  if (param.size() < 1 || param[0] == '#')
    return param;
  return "#" + param;
}

std::string IRC_request_parser::DelChanPrefixFromParam(
  const std::string& param) {
  std::string ret = param;
  while (ret[0] == '#')
    ret = ret.substr(1);
  return ret;
}


enum IRCCommand IRC_request_parser::ConvertStrToCom(const std::string& comm) {
  if (comm == "CAP")
    return CAP;
  else if (comm == "PASS")
    return PASS;
  else if (comm == "USER")
    return USER;
  else if (comm == "NICK")
    return NICK;
  else if (comm == "MOTD")
    return MOTD;
  else if (comm == "PING")
    return PING;
  // TODO(kyungjle) 상용 서버들이 소문자 커맨드를 어떻게 처리하는지 확인
  // TODO(kyungjle) irssi 클라이언트가 왜 motd를 소문자로 보내는지 확인
  else if (comm == "motd")
    return MOTD;
  else if (comm == "QUIT")
    return QUIT;
  else if (comm == "JOIN")
    return JOIN;
  else if (comm == "NAMES")
    return NAMES;
  else if (comm == "MODE")
    return MODE;
  else if (comm == "PART")
    return PART;
  else if (comm == "TOPIC")
    return TOPIC;
  else if (comm == "KICK")
    return KICK;
  else if (comm == "PRIVMSG")
    return PRIVMSG;
  else if (comm == "INVITE")
    return INVITE;
  else
    return UNKNOWN;
}

std::string IRC_request_parser::ConvertComToStr(enum IRCCommand comm) {
  if (comm == CAP)
    return "CAP";
  else if (comm == PASS)
    return "PASS";
  else if (comm == USER)
    return "USER";
  else if (comm == NICK)
    return "NICK";
  else if (comm == QUIT)
    return "QUIT";
  else if (comm == MOTD)
    return "MOTD";
  else if (comm == JOIN)
    return "JOIN";
  else if (comm == NAMES)
    return "NAMES";
  else if (comm == MODE)
    return "MODE";
  else if (comm == PART)
    return "PART";
  else if (comm == TOPIC)
    return "TOPIC";
  else if (comm == KICK)
    return "KICK";
  else if (comm == PRIVMSG)
    return "PRIVMSG";
  else if (comm == INVITE)
    return "INVITE";
  else
    return "";
}
