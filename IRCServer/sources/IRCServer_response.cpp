#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

std::string IRCServer::MakeResponse(IRCContext& context)
{
	std::string command;
	if (context.command == UNKNOWN)
		command = context.rawMessage;
	else
		command = IRCContext::ConvertCommandToStr(context.command);
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;

	// error response
	if (400 <= context.numericResult && context.numericResult < 600)
	{
		result << ":" + _serverName + " "  << std::setw(3) << std::setfill('0') 
			<< context.numericResult << " ";
		switch (context.numericResult)
		{
			// ERR_NONICKNAMEGIVEN
			case 431:
				result << clientNickname << " :No nickname given";
				break ;
			// ERR_ERRONEUSNICKNAME
			case 432:
				result << clientNickname << " :Erroneus nickname";
				break ;
			// ERR_NICKNAMEINUSE
			case 433:
				result << clientNickname << " :Nickname is already in use";
				break ;
			// ERR_NOTREGISTERED
			case 451:
				result << clientNickname << " :You have not registered";
				break ;
			// ERR_NEEDMOREPARAMS
			case 461:
				result << clientNickname << " " << command << " :Not enough parameters";
				break ;
			// ERR_ALREADYREGISTERED
			case 462:
				result << clientNickname << " :You may not reregister";
				break ;
			// ERR_PASSWDMISMATCH
			case 464:
				result << clientNickname << " :Password incorrect";
				break ;
			// ERR_UNKNOWNCOMMAND
			case 421:
				// FALLTHROUGH
			default:
				result << clientNickname << " " << command << " :Unknown command";
				break ;
		}
	}
	// normal result
	else
	{
		// source ::=  <servername> / ( <nickname> [ "!" <user> ] [ "@" <host> ] )
		// can omit username and hostname for client source
		if (context.source.size() == 0)
			result << ":" << _serverName << " ";
		else
			result << ":" << context.source << " ";
		
		/*
		notes on IRCContext, IRCServer::MakeResponse

		MakeResponse를 호출하는 메소드는 IRCContext의 stringResult 필드에 
		올바른 응답 메시지를 모두 작성해놔야 합니다. (예외상황 제외)
		*/ 
		if (context.numericResult > 0)
			result << std::setw(3) << std::setfill('0') << context.numericResult << " ";
		result << context.stringResult;
	}

	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: MakeResponse: from context" << context;
	std::cout << "[DEBUG] IRCServer: MakeResponse: response " << result.str() << std::endl;
	# endif

	result << "\r\n";
	return result.str();
}
