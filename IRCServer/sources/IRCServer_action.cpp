#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

# ifndef VERSION
# define VERSION "42.42"
# endif

void IRCServer::ActionAcceptClient(IRCContext& context)
{
	if (context.command == CAP)
		return ;
	
	if (context.client->GetStatus() == REGISTER_PENDING)
	{
		if (context.command == NICK || context.command == USER
			|| context.params.size() != 1 || context.params[0] != _serverPass)
			throw IRCError::WrongPassword();
		if (context.command == PASS)
			context.client->SetStatus(REGISTER_ONGOING);
		return ;
	}

	if (context.command == PASS)
		throw IRCError::NotRegistered();
	if (context.command == NICK)
	{
		if (context.params.size() == 0)
			throw IRCError::NoNickname();
		else if (context.params.size() > 1)
			throw IRCError::WrongNickname();
		// TODO validate nickname
		context.client->SetNickName(context.params[0]);
	}
	if (context.command == USER)
	{
		// TODO USER command
		context.client->SetHostName(context.params[0]);
	}

	if (context.client->GetStatus() == REGISTERED)
	{
		if (context.command == NICK)
		{
			// TODO broadcast nickname change
		}

		return ;
	}
	_clients[context.client->GetNickname()] = context.client;
	context.client->SetStatus(REGISTERED);
	std::stringstream stringresult;
	std::string clientNickname = context.client->GetNickname();

	// RPL_WELCOME
	stringresult.str("");
	context.stringResult.clear();
	stringresult << clientNickname
		<< " :Welcome to the "<< _serverName << " Network, " << clientNickname << "!";
	context.numericResult = 1;
	context.stringResult = stringresult.str();
	context.client->Send(MakeResponse(context));

	// RPL_YOURHOST
	stringresult.str("");
	context.stringResult.clear();
	stringresult << clientNickname
		<< " :Your host is "<< _serverName << ", running version " << VERSION;
	context.numericResult = 2;
	context.stringResult = stringresult.str();
	context.client->Send(MakeResponse(context));

	// RPL_CREATED
	stringresult.str("");
	context.stringResult.clear();
	stringresult << clientNickname
		<< " :This server was created "<< _startDate;
	context.numericResult = 3;
	context.stringResult = stringresult.str();
	context.client->Send(MakeResponse(context));

	// RPL_MYINFO
	// TODO set RPL_MYINFO
	stringresult.str("");
	context.stringResult.clear();
	stringresult << clientNickname << " " << _serverName << " " << VERSION << " r oitlk";
	context.stringResult = stringresult.str();
	context.numericResult = 4;
	context.client->Send(MakeResponse(context));

	// RPL_ISUPPORT
	// TODO set RPL_ISUPPORT
	stringresult.str("");
	context.stringResult.clear();
	stringresult << clientNickname << " CHANTYPES=# CASEMAPPING=ascii CHANMODES=o,kl,,it  :are supported by this server";
	context.stringResult = stringresult.str();
	context.numericResult = 5;
	context.client->Send(MakeResponse(context));

	// MOTD
	ActionMOTD(context);

	context.FDsPendingWrite.insert(context.client->GetFD());
}

/**************/

void IRCServer::ActionMOTD(IRCContext& context)
{
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;

	result << "375 " 
		<< clientNickname << " : -- Welcome to " << _serverName << "-- \r\n";
	result << ":" + _serverName + " 372 " 
		<< clientNickname << " :Mesasge of the day:\r\n";
	result << ":" + _serverName + " 372 " 
		<< clientNickname << " :do ft_irc not webserv.\r\n";
	result << ":" + _serverName + " 376 " 
		<< clientNickname << " :end of MOTD";
	
	context.numericResult = -1;
	context.stringResult = result.str();
	context.client->Send(MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}

void IRCServer::ActionPING(IRCContext& context)
{
	std::stringstream result;

	result << ":" + _serverName << " PONG " << _serverName;
	for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++)
	{

	}
}
