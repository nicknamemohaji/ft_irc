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

/*
Action handler for CAP, PASS, USER, NICK commands;
*/
void IRCServer::ActionAcceptClient(IRCContext& context)
{
	// ignore CAP message to disable capability negotiation settings
	if (context.command == CAP)
		return ;
	
	// requires password before registering
	if (context.client->GetStatus() == REGISTER_PENDING)
	{
		if (context.command != PASS)
			throw IRCError::WrongPassword();
		if (context.params.size() != 1 || context.params[0] != _serverPass)
			throw IRCError::WrongPassword();
		context.client->SetStatus(REGISTER_PASS);
		return ;
	}

	// dont accept PASS message after correct password
	if (context.command == PASS)
	{
		if (context.client->GetStatus() != REGISTER_PENDING)
			throw IRCError::NotRegistered();
		else
			throw IRCError::AlreadyRegistered();
	}
	
	if (context.command == NICK)
	{
		// validate parameter
		if (context.params.size() == 0)
			throw IRCError::NoNickname();
		else if (context.params.size() > 1)
			throw IRCError::WrongNickname();
		std::string new_name = context.params[0];
		if (GetClient(new_name) != NULL)
			throw IRCError::ExitstingNickname();
		// validate nickname
		// TODO set RPL_ISUPPORT for nickname rules
		if (!(2 <= new_name.size() && new_name.size() <= 30))
			throw IRCError::WrongNickname();
		for (unsigned int i = 0; i < new_name.size(); i++)
		{
			if (!std::isalnum(static_cast<unsigned char>(new_name[i])))
				throw IRCError::WrongNickname();
		}
	
		// good to go!
		if (context.client->GetStatus() >= REGISTERED)
		{
			std::string prev_name = context.client->GetNickname();

			// send acknowledgement to user
			context.createSource = true;
			context.numericResult = -1;
			context.stringResult = "NICK " + new_name;
			context.client->Send(MakeResponse(context));
			context.FDsPendingWrite.insert(context.client->GetFD());
			// TODO IRCClient::GetChannels will return std::vector<std::string> in the future
			IRCClientJoinedChannels channels = context.client->ListChannels();
			for (IRCClientJoinedChannels::iterator it = channels.begin(); it != channels.end(); it++)
			{
				// broadcast
				context.channel = it->second;
				SendMessageToChannel(context, SendToAllExceptMe);
				// change name from channel
				context.channel->DelChannelUser(prev_name);
				context.channel->AddChannelUser(new_name);
			}
			
			// remove previous nickname
			_clients.erase(_clients.find(context.client->GetNickname()));
		}

		// Add new name
		_clients[new_name] = context.client;
		context.client->SetNickName(new_name);
		if (context.client->GetStatus() >= REGISTERED)
			return ;
	}
	if (context.command == USER)
	{
		if (context.params.size() != 4)
			throw IRCError::MissingParams();
		
		context.client->SetUserName(context.params[0]);
		return ;
	}

	/****** registration complete *******/
	std::string clientNickname = context.client->GetNickname();
	context.client->SetStatus(REGISTERED);
	std::stringstream strstream;

	// send RPL_WELCOME
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname
		<< " :Welcome to the "<< _serverName << " Network, " << clientNickname << "!";
	context.numericResult = 1;
	context.stringResult = strstream.str();
	context.client->Send(MakeResponse(context));

	// send RPL_YOURHOST
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname
		<< " :Your host is "<< _serverName << ", running version " << VERSION;
	context.numericResult = 2;
	context.stringResult = strstream.str();
	context.client->Send(MakeResponse(context));

	// send RPL_CREATED
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname
		<< " :This server was created "<< _startDate;
	context.numericResult = 3;
	context.stringResult = strstream.str();
	context.client->Send(MakeResponse(context));

	// send RPL_MYINFO
	// TODO set RPL_MYINFO
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname << " " << _serverName << " " << VERSION << " r oitlk";
	context.stringResult = strstream.str();
	context.numericResult = 4;
	context.client->Send(MakeResponse(context));

	// send RPL_ISUPPORT
	// TODO set RPL_ISUPPORT
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname << " CHANTYPES=# CASEMAPPING=ascii CHANMODES=o,kl,,it "
		<< "PREFIX=(o)@ NICKLEN=30 :are supported by this server";
	context.stringResult = strstream.str();
	context.numericResult = 5;
	context.client->Send(MakeResponse(context));

	// send MOTD
	ActionMOTD(context);

	context.FDsPendingWrite.insert(context.client->GetFD());
}

/**************/

void IRCServer::ActionMOTD(IRCContext& context)
{
	if (context.command == MOTD)
	{
		for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++)
		{
			context.stringResult = *it;
			if (*it != _serverName)
				throw IRCError::NoSuchServer();
		}
	}
	std::string clientNickname = context.client->GetNickname();
	
	context.stringResult = clientNickname + " : -- Welcome to " + _serverName + "--";
	context.numericResult = 375;
	context.client->Send(MakeResponse(context));
	
	context.stringResult = clientNickname + " :Mesasge of the day:";
	context.numericResult = 372;
	context.client->Send(MakeResponse(context));
	
	context.stringResult = clientNickname + " :WeLOve42Seoul";
	context.numericResult = 372;
	context.client->Send(MakeResponse(context));

	context.stringResult = clientNickname + " :end of MOTD";
	context.numericResult = 376;
	context.client->Send(MakeResponse(context));

	context.FDsPendingWrite.insert(context.client->GetFD());
}
