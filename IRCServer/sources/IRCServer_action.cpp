#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::Context(IRCContext& context)
{
	// registration
	if (context.client->GetStatus() != REGISTERED)
	{
		if (context.client->GetStatus() == REGISTER_PENDING)
		{
			switch (context.command)
			{
				case PASS:
					if (context.params[0] != _serverPass)
						throw IRCError::WrongPassword();
					else
						return context.client->Context(context);
				case CAP:
					return ;
				case USER:
					// FALLTHROUGH
				case NICK:
					// registration without PASS
					throw IRCError::WrongPassword();
				default:
					// not registered
					throw IRCError::NotRegistered();
			}
		}
		else
		{
			switch (context.command)
			{
				case PASS:
					// already validated command
					throw IRCError::AlreadyRegistered();
				case CAP:
					// do nothing for CAP negotiation
					return ;
				case USER:
					context.client->Context(context);
					break ;
				case NICK:
					// TODO validate nickname
					context.client->Context(context);
					break ;
				default:
					// not registered
					throw IRCError::NotRegistered();
			}

			// if (context.client->GetStatus() == REGISTERED)
				AcceptClient(context);
			return ;
		}
	}

	// other...
	switch (context.command)
	{
		case PASS:
			// FALLTHROUGH
		case CAP:
			// FALLTHROUGH
		case USER:
			// FALLTHROUGH
		case NICK:
			// FALLTHROUGH
			throw IRCError::AlreadyRegistered();
		case MOTD:
			context.client->Send(ManageMOTD(context));
			break ;
		default:
			context.client->Send(":" + _serverName + " 465 :You are banned\r\n");
	}
	context.FDsPendingWrite.insert(context.client->GetFD());
}

/**************/

void IRCServer::AcceptClient(IRCContext& context)
{
	// RPL_WELCOME
	context.numericResult = 1;
	context.client->Send(MakeResponse(context));
	// RPL_YOURHOST
	context.numericResult = 2;
	context.client->Send(MakeResponse(context));
	// RPL_CREATED
	context.numericResult = 3;
	context.client->Send(MakeResponse(context));
	// RPL_MYINFO
	context.numericResult = 4;
	context.client->Send(MakeResponse(context));
	// RPL_ISUPPORT
	context.numericResult = 5;
	context.client->Send(MakeResponse(context));
	// MOTD
	context.client->Send(ManageMOTD(context));

	context.FDsPendingWrite.insert(context.client->GetFD());
}

/**************/

std::string IRCServer::ManageMOTD(IRCContext& context)
{
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;

	result << ":" + _serverName + " 375 " 
		<< clientNickname << " : -- Welcome to " << _serverName << "-- \r\n";
	result << ":" + _serverName + " 372 " 
		<< clientNickname << " :Mesasge of the day:\r\n";
	result << ":" + _serverName + " 372 " 
		<< clientNickname << " :do irc than webserv.\r\n";
	result << ":" + _serverName + " 376 " 
		<< clientNickname << " :end of MOTD\r\n";
	
	return result.str();
}