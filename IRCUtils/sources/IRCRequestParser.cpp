#include "IRCRequestParser.hpp"
#include "IRCTypes.hpp"

#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

#include "IRCErrors.hpp"

static std::string	_BufferParseUntilSpace(Buffer& message);
static void			_BufferRemoveSpace(Buffer& message);
bool 				IRCRequestParser::ParseMessage(
						Buffer& message, IRCCommand& command, IRCParams& params);
void 				IRCRequestParser::AddNewLineToBuffer(Buffer& buffer);
std::string 		IRCRequestParser::AddChanPrefixToParam(const std::string& param);
std::string 		IRCRequestParser::DelChanPrefixFromParam(const std::string& param);
enum IRCCommand 	IRCRequestParser::ConvertStrToCom(const std::string& command);
std::string 		IRCRequestParser::ConvertComToStr(enum IRCCommand command);


static std::string _BufferParseUntilSpace(Buffer& message)
{
	// find
	Buffer::iterator it_SP = std::find(message.begin(), message.end(), ' ');
	Buffer::iterator it_CR = std::find(message.begin(), message.end(), '\r');
	if (it_SP == message.end() && it_CR == message.end())
		throw IRCError::UnknownCommand();
	
	// compare SP and CR position
	std::string ret;
	if (std::distance(message.begin(), it_SP) < std::distance(message.begin(), it_CR))
	{
		// space is first occurance
		ret.assign(message.begin(), it_SP);
		message.erase(message.begin(), it_SP);
	}
	else
	{
		// carriage return is first occurance
		ret.assign(message.begin(), it_CR);
		message.erase(message.begin(), it_CR);
	}

	return ret;
}

// SPACE ::= %x20 *(%x20)
// so should handle multiple space characters
static void _BufferRemoveSpace(Buffer& message)
{
	while (message.begin() != message.end() && *(message.begin()) == ' ')
		message.erase(message.begin());
}

bool IRCRequestParser::ParseMessage(Buffer& message, IRCCommand& command, IRCParams& params)
{
	// 1. ['@' <tags> SPACE]
	// tags can be disabled by CAP negotiation, and our server will disable it.

	// 2. [':' <source> SPACE ]
	// client MUST NOT send source to server
	if (message[0] == ':')
		throw IRCError::UnknownCommand();

	// 3. <command>
	{
		std::string _raw_command = _BufferParseUntilSpace(message);
		_BufferRemoveSpace(message);

		command = IRCRequestParser::ConvertStrToCom(_raw_command);
	}

	// 4. <parameters> <crlf>
	while (*(message.begin()) != '\r')
	{
		// parameters	::= *( SPACE middle ) [ SPACE ":" trailing ]
		// middle		::=  nospcrlfcl *( ":" / nospcrlfcl )
		// trailing		::=  *( ":" / " " / nospcrlfcl )
		Buffer::iterator it;
		std::string _param;

		// trailing parameter
		if (message[0] == ':')
		{
			it = std::find(message.begin(), message.end(), '\r');
			// ignore first `:`
			_param.assign(message.begin() + 1, it);
			// remove sliced message
			message.erase(message.begin(), it);
		}
		// middle parameter
		else
			_param = _BufferParseUntilSpace(message);
		
		// add params to context
		params.push_back(_param);
		_BufferRemoveSpace(message);
	}

	// remove CRLF from buffer to handle multiline message
	if (message.begin() != message.end() && *(message.begin()) == '\r')
		message.erase(message.begin());
	if (message.begin() != message.end() && *(message.begin()) == '\n')
		message.erase(message.begin());

	return true;
}


void IRCRequestParser::AddNewLineToBuffer(Buffer& buffer)
// requires CRLF
{
	// check for CR
	Buffer::const_iterator it = std::find(buffer.begin(), buffer.end(), '\r');
	// CR is not set
	if (it == buffer.end())
	{
		// check for LF
		Buffer::const_iterator it2 = std::find(buffer.begin(), buffer.end(), '\n');
		// if both CR-LF is not set, then message is incomplete
		if (it2 == buffer.end())
			return ;
		// else append CR in front of LF to match set
		buffer.insert(it2, '\r');
	}
	// CR is set
	else
	{
		// check for LF
		if ((it + 1) == buffer.end() || *(it + 1) != '\n')
		{
			// append LF after CR to match set
			buffer.insert(it + 1, '\n');
		}
	}
	/*
	notes on inserting CR or LF:

	ircv3 표준과 modern irc 문서는 호환성을 위해 단일 LF(\n)만 오는 메시지 처리를 권장하고(SHOULD), 
	단일 CR(\r)만 오는 메시지 처리를 제안합니다(MAY). 
	(Servers SHOULD handle single \n character, and MAY handle a single \r character, as if it was a \r\n pair)

	RFC 1459에서도 호환성과 관련해 단일 LF를 인식하는 서버가 있다고 언급하고 있지만, 이에 대한 요구사항은 없습니다.

	이 구현은 modern irc 표준을 따르므로, 단일 LF 또는 CR이 올 경우에 CRLF 짝을 맞추도록 추가하였습니다.
	*/
}

IRCParams IRCRequestParser::SeparateParam(const std::string& param, const std::string& delim)
{
    IRCParams ret;
    size_t start = 0;
    size_t end = param.find(delim);
    
    while (end != std::string::npos)
    {
        ret.push_back(param.substr(start, end - start));
        start = end + delim.length();
        end = param.find(delim, start);
    }
    
    // 마지막 요소 추가
    ret.push_back(param.substr(start));
    
    return ret;
}

std::string IRCRequestParser::AddChanPrefixToParam(const std::string& param)
{
	if(param.size() < 1 || param[0] == '#')
		return param;
	return "#" + param;
}

std::string IRCRequestParser::DelChanPrefixFromParam(const std::string& param)
{
	std::string ret = param;
	while (ret[0] == '#')
		ret = ret.substr(1);
	return ret;
}


enum IRCCommand IRCRequestParser::ConvertStrToCom(const std::string& command)
{
	if (command == "CAP")
		return CAP;
	else if (command == "PASS")
		return PASS;
	else if (command == "USER")
		return USER;
	else if (command == "NICK")
		return NICK;
	else if (command == "MOTD")
		return MOTD;
	else if (command == "PING")
		return PING;
	else if (command == "motd")	// WTF
		return MOTD;
	else if (command == "QUIT")
		return QUIT;
	else if (command == "JOIN")
		return JOIN;
	else if (command == "NAMES")
		return NAMES;
	else if (command == "MODE")
		return MODE;
	else if (command == "PART")
		return PART;
	else if (command == "TOPIC")
		return TOPIC;
	else if (command == "KICK")
		return KICK;
	else if (command == "PRIVMSG")
		return PRIVMSG;
	else if (command == "INVITE")
		return INVITE;
	else
		throw IRCError::UnknownCommand();
}

std::string IRCRequestParser::ConvertComToStr(enum IRCCommand command)
{
	if (command == CAP)
		return "CAP";
	else if (command == PASS)
		return "PASS";
	else if (command == USER)
		return "USER";
	else if (command == NICK)
		return "NICK";
	else if (command == QUIT)
		return "QUIT";
	else if (command == MOTD)
		return "MOTD";
	else if (command == JOIN)
		return "JOIN";
	else if (command == NAMES)
		return "NAMES";
	else if (command == MODE)
		return "MODE";
	else if (command == PART)
		return "PART";
	else if (command == TOPIC)
		return "TOPIC";
	else if (command == KICK)
		return "KICK";
	else if (command == PRIVMSG)
		return "PRIVMSG";
	else if (command == INVITE)
		return "INVITE";
	else
		throw IRCError::UnknownCommand();
}
