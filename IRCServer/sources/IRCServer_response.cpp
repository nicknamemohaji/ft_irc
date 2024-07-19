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

# ifndef BUILDDATE
# define BUILDDATE "2024-07-19"
# endif
# ifndef VERSION
# define VERSION "42.42"
# endif


std::string IRCServer::MakeResponse(IRCContext& context)
{
	std::string command;
	if (context.command == UNKNOWN)
		command = context.rawMessage;
	else
		command = IRCContext::ConvertCommandToStr(context.command);
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;
	result << std::setw(3) << std::setfill('0');
	// TODO use tag

	// error response
	if (400 <= context.numericResult && context.numericResult < 600)
	{
		result << ":" + _serverName + " " << context.numericResult << " ";
		switch (context.numericResult)
		{

			// ERR_NOTREGISTERED
			case 433:
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
		// normal non-numeric result
		if (context.numericResult == -1)
		{
			switch (context.command)
			{
				case MOTD:
					return ManageMOTD(context);
				default:
					result << clientNickname << " " << command << " :Unknown command";
					break ;
			}
		}
		// normal numeric result
		else
		{
			switch (context.numericResult)
			{
				// TODO RPL_MYINFO, RPL_ISUPPORT
				// RPL_WELCOME
				case 1:
					result << clientNickname
						<< " ::Welcome to the "<< _serverName << " Network, "
						<< clientNickname << "!" << context.client->GetHostName();
					break ;
				// RPL_YOURHOST
				case 2:
					result << clientNickname
						<< " :Your host is "<< _serverName <<", running version " VERSION;
					break ;
				// RPL_CREATED
				case 3:
					result << clientNickname << " :This server was created " BUILDDATE;
					break ;
				// RPL_MYINFO
				case 4:
					result << clientNickname << " :";
					break ;
				// RPL_ISUPPORT
				case 5:
					result << clientNickname << " :";
					break ;
			}
		}
	}

	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: MakeResponse: from context" << context;
	std::cout << "[DEBUG] IRCServer: MakeResponse: response " << result.str() << std::endl;
	# endif

	result << "\r\n";
	return result.str();
}
