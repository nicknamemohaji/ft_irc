#include <iostream>
#include <string>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

static std::string	_BufferParseUntilSpace(Buffer& message);
static void			_BufferRemoveSpace(Buffer& message);

static std::string _BufferParseUntilSpace(Buffer& message)
{
	// find
	Buffer::iterator it = std::find(message.begin(), message.end(), ' ');
	if (it == message.begin())
		throw IRCError::UnknownCommand();
	if (it == message.end())
	{
		it = std::find(message.begin(), message.end(), '\r');
		if (it == message.end())
			throw IRCError::UnknownCommand();
	}
	
	// slice
	std::string ret;
	ret.assign(message.begin(), it);

	// remove sliced message from IRCClient::recvBuf
	message.erase(message.begin(), it);

	return ret;
}

// SPACE ::= %x20 *(%x20)
// so should handle multiple space characters
static void _BufferRemoveSpace(Buffer& message)
{
	while (message.begin() != message.end() && *(message.begin()) == ' ')
		message.erase(message.begin());
	if (message.begin() == message.end())
		throw IRCError::UnknownCommand();
}

bool IRCServer::RequestParser(Buffer& message, IRCContext& context)
{

	// requires CRLF
	{
		// check for CR
		Buffer::iterator it = std::find(message.begin(), message.end(), '\r');
		// CR is not set
		if (it == message.end())
		{
			// check for LF
			Buffer::iterator it2 = std::find(message.begin(), message.end(), '\n');
			// if both CR-LF is not set, then message is incomplete
			if (it2 == message.end())
				return false;
			// else append CR in front of LF to match set
			message.insert(it2, '\r');
		}
		// CR is set
		else
		{
			// check for LF
			if ((it + 1) == message.end() || *(it + 1) != '\n')
			{
				// append LF after CR to match set
				message.insert(it + 1, '\n');
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

	// 1. ['@' <tags> SPACE]
	// tags can be disabled by CAP negotiation, and our server will disable it.

	// 2. [':' <source> SPACE ]
	// client MUST NOT send source to server
	if (message[0] == ':')
		throw IRCError::UnknownCommand();

	// 3. <command>
	{
		std::string command = _BufferParseUntilSpace(message);
		_BufferRemoveSpace(message);

		context.command = IRCContext::ConvertStrToCommand(command);
	}

	// 4. <parameters> <crlf>
	while (*message.begin() != '\r')
	{
		// parameters	::= *( SPACE middle ) [ SPACE ":" trailing ]
		// middle		::=  nospcrlfcl *( ":" / nospcrlfcl )
		// trailing		::=  *( ":" / " " / nospcrlfcl )
		Buffer::iterator it;
		std::string param;

		// check for colon
		if (message[0] == ':')
		{
			// trailing parameter
			it = std::find(message.begin(), message.end(), '\r');
			// ignore first `:`
			param.assign(message.begin() + 1, it);
		}
		else
		{
			// middle parameter
			it = std::find(message.begin(), message.end(), ' ');
			// but can have only middle parameters (trailing is optional)
			if (it == message.end())
				it = std::find(message.begin(), message.end(), '\r');
			param.assign(message.begin(), it);
		}
		// add params to context
		context.params.push_back(param);
		// remove sliced message from IRCClient::recvBuf
		message.erase(message.begin(), it);
		_BufferRemoveSpace(message);

		std::cout << "param [" << param << "]\n";
		std::cout << "[" << message << "]\n";
	}
	// remove CRLF from buffer so that next message does not get deleted...
	message.erase(message.begin(), message.begin() + 2);
	
	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: RequestParser: parse result" << context;
	# endif

	return true;
}
