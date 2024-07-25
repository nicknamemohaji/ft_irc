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

void IRCServer::ActionJOIN(IRCContext& context){
	if(context.params.size() < 1)
		return ; // ERR_NEEDMOREPARAMS (461)
	//채널이 서버에 있는지 확인하는 구간. server에 channel확인 메소드 추가.
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;
	result << ":" << clientNickname << std::endl;
	for(unsigned int i = 0; i < context.params.size(); i++)
	{
		std::cout << context.params[i] << std::endl;
	}
}

/*
notes on IRCServer::Actions:

Actions는 멤버 함수 배열입니다. Actions에 저장되는 순서는 enum IRCCommand를 사용합니다.

각 메소드는 필요시 IRCContext::client를 참조해 클라이언트에게 메시지를 보낼 수 있는데(IRCClient::Send),
이 때 IRCContext::FDsPendingWrite에 IRCClient->GetFD의 결과를 추가해야만 정상적으로 메시지가 전송됩니다.

ex)
context.client->Send(MakeResponse(context));
context.FDsPendingWrite.insert(context.client->GetFD());
*/
