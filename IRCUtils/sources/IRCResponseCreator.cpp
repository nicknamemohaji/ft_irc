#include "IRCUtils/includes/IRCResponseCreator.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include "IRCServer/includes/IRCChannel.hpp"
#include "IRCServer/includes/IRCClient.hpp"
#include "IRCServer/includes/IRCContext.hpp"
#include "IRCServer/includes/IRCServer.hpp"
#include "IRCServer/includes/IRCTypes.hpp"
#include "IRCUtils/includes/IRCRequestParser.hpp"

/**
 * @brief IRCContext로부터 완전한 메시지를 생성합니다
 *
 * @param[in] context 메시지를 생성하기 위한 정보
 *
 * @return source-command-parameter로 구성된 IRC 메시지
 *
 * @warning context에는 client, server 포인터가 설정돼 있어야 합니다
 *
 * @note context.createSource가 true이면 source에 클라이언트 정보를,
 * false이면 서버 정보를 사용합니다
 *
 * @note context.numericResult가 -1이면 커맨드를 문자열로 변환한 값을,
 * -1이 아니면 numericResult에 주어진 값을 사용합니다
 */
std::string IRC_response_creator::MakeResponse(const IRCContext& context) {
  std::stringstream result;

  // 1. [':' <source> SPACE]
  // source ::=  <servername> / ( <nickname> [ "!" <user> ] [ "@" <host> ] )
  if (context.createSource) {
    result << ":" << context.client->GetNickname() << "!"
           << context.client->GetUserName() << "@" << context.client->GetIP()
           << " ";
  } else {
    result << ":" << context.server->GetServerName() << " ";
  }

  // 2. <command>
  // command ::=  letter* / 3digit
  if (context.numericResult == -1)
    result << IRC_request_parser::ConvertComToStr(context.command) << " ";
  else
    result << std::setw(3) << std::setfill('0') << context.numericResult << " ";

  // 3. <parameters>
  // TODO(kyungjle): move error response creator to caller
  if (400 <= context.numericResult && context.numericResult < 600) {
    // error response
    std::string command = IRC_request_parser::ConvertComToStr(context.command);
    std::string clientNickname = context.client->GetNickname();
    switch (context.numericResult) {
      // ERR_NOSUCHNICK
      case 401:
        result << clientNickname << " " << context.stringResult
               << " :No such nick/channel";
        break;
      // ERR_NOSUCHCHANNEL
      case 403:
        result << clientNickname << " " << context.stringResult
               << " :No such channel";
        break;
      // ERR_CANNOTSENDTOCHAN
      case 404:
        result << clientNickname << " " << context.stringResult
               << " :Cannot send to channel or user";
        break;
      // ERR_TOOMANYCHANNELS
      case 405:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :You have joined too many channels";
        break;
      // ERR_USERNOTINCHANNEL
      case 441:
        result << clientNickname << " " << context.stringResult
               << " :They aren't on that channel";
        break;
      // ERR_NOTONCHANNEL (442) 채널에 유저가 존재하지 않음
      case 442:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :You're not on that channel";
        // "<client> <channel> :You're not on that channel"
        break;
      // ERR_USERONCHANNEL
      case 443:
        result << clientNickname << " " << context.stringResult << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :is already on channel";
        break;
      // *ERR_CHANNELISFULL* (471) 채널 포화상태
      case 471:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :Cannot join channel (+l)";
        break;
      // *ERR_INVITEONLYCHAN* (473) 인바이트 전용채널, 인바이트 안된상태
      case 472:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :is unknown mode char to me";
        break;
      // *ERR_UNKNOWNMODE* (472) 서버에서 인식할 수 없는 모드문자 사용
      case 473:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :Cannot join channel (+i) - you must be invited";
        break;
      // ERR_BADCHANNELKEY* (475) 비밀번호 다름
      case 475:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :Cannot join channel (+k)";
        break;
      // *ERR_BADCHANMASK* (476) 채널이름이 유효하지 않음
      case 476:
        result << context.stringResult << " :Bad Channel Mask";
        break;
      // ERR_CHANOPRIVSNEEDED
      case 482:
        result << clientNickname << " "
               << context.channel->GetChannelInfo(kChannelName)
               << " :You're not a channel operator";
        break;

      // ERR_UNKNOWNCOMMAND
      case 421:
        // FALLTHROUGH
      default:
        result << clientNickname << " " << context.stringResult
               << " :Unknown command";
        break;
    }
  } else {
    // normal response
    result << context.stringResult;
  }

  // 4. <crlf>
  result << "\r\n";
  return result.str();
}

void IRC_response_creator::ErrorSender(IRCContext context,
                                       unsigned int errornum) {
  context.numericResult = errornum;
  context.createSource = false;
  context.client->Send(IRC_response_creator::MakeResponse(context));
  context.pending_fds->insert(context.client->GetFD());
}

/**
 * @brief ERR_NOSUCHSERVER (402) 에러를 전송합니다
 */
void IRC_response_creator::ERR_NOSUCHSERVER(IRCClient* client,
                                            const std::string& server_name,
                                            FDSet* pending_fds,
                                            const std::string& param) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 402 " << client->GetNickname() << " "
              << param << " :No such server\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_NONICKNAMEGIVEN (431) 에러를 전송합니다
 */
void IRC_response_creator::ERR_NONICKNAMEGIVEN(IRCClient* client,
                                               const std::string& server_name,
                                               FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 432 " << client->GetNickname()
              << " :No nickname given\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_ERRONEUSNICKNAME (432) 에러를 전송합니다
 */
void IRC_response_creator::ERR_ERRONEUSNICKNAME(IRCClient* client,
                                                const std::string& server_name,
                                                FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 432 " << client->GetNickname()
              << " :Erroneus nickname\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_NICKNAMEINUSE (433) 에러를 전송합니다
 */
void IRC_response_creator::ERR_NICKNAMEINUSE(IRCClient* client,
                                             const std::string& server_name,
                                             FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 433 " << client->GetNickname()
              << " :Nickname is already in use\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_NOTREGISTERED (451) 에러를 전송합니다
 */
void IRC_response_creator::ERR_NOTREGISTERED(IRCClient* client,
                                             const std::string& server_name,
                                             FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 451 " << client->GetNickname()
              << " :You have not registered\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_NEEDMOREPARAMS(461) 에러를 전송합니다
 */
void IRC_response_creator::ERR_NEEDMOREPARAMS(IRCClient* client,
                                              const std::string& server_name,
                                              FDSet* pending_fds,
                                              enum IRCCommand command) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 461 " << client->GetNickname()
              << IRC_request_parser::ConvertComToStr(command) << " "
              << " :Not enough parameters\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_ALREADYREGISTERED (462) 에러를 전송합니다
 */
void IRC_response_creator::ERR_ALREADYREGISTERED(IRCClient* client,
                                                 const std::string& server_name,
                                                 FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 462 " << client->GetNickname()
              << " :You may not reregister\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}

/**
 * @brief ERR_PASSWDMISMATCH(464) 에러를 전송합니다
 */
void IRC_response_creator::ERR_PASSWDMISMATCH(IRCClient* client,
                                              const std::string& server_name,
                                              FDSet* pending_fds) {
  std::stringstream err_message;
  err_message << ":" << server_name << " 464 " << client->GetNickname()
              << " :Password incorrect\r\n";

  client->Send(err_message.str());
  pending_fds->insert(client->GetFD());
}
