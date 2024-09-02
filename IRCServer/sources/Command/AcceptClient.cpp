#include "IRCServer.hpp"
#include "IRCClient.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCTypes.hpp"
#include "IRCContext.hpp"
#include "IRCChannel.hpp"
#include "IRCErrors.hpp"
#include "IRCResponseCreator.hpp"

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
    if (context.command != PASS ||
        !(context.params.size() == 1 && context.params[0] == _serverPass)) {
      return IRC_response_creator::ERR_PASSWDMISMATCH(
          context.client, _serverName, context.pending_fds);
    }
		context.client->SetStatus(REGISTER_PASS);
		return ;
	}

	// dont accept PASS message after correct password
	if (context.command == PASS)
	{
    if (context.client->GetStatus() != REGISTER_PENDING) {
      return IRC_response_creator::ERR_NOTREGISTERED(
          context.client, _serverName, context.pending_fds);
    } else {
      return IRC_response_creator::ERR_ALREADYREGISTERED(
          context.client, _serverName, context.pending_fds);
    }
	}
	
	if (context.command == NICK)
	{
		// validate parameter
    if (context.params.size() == 0) {
      return IRC_response_creator::ERR_NONICKNAMEGIVEN(
          context.client, _serverName, context.pending_fds);
    } else if (context.params.size() > 1) {
      return IRC_response_creator::ERR_ERRONEUSNICKNAME(
          context.client, _serverName, context.pending_fds);
    }
		std::string new_name = context.params[0];
		if (GetClient(new_name) != NULL)
    {
       return IRC_response_creator::ERR_NICKNAMEINUSE(
          context.client, _serverName, context.pending_fds);
    }
		// validate nickname
		// TODO set RPL_ISUPPORT for nickname rules
		if (!(2 <= new_name.size() && new_name.size() <= 30)){
      return IRC_response_creator::ERR_ERRONEUSNICKNAME(
          context.client, _serverName, context.pending_fds);
    }
		for (unsigned int i = 0; i < new_name.size(); i++)
		{
      if (!std::isalnum(static_cast<unsigned char>(new_name[i]))) {
        return IRC_response_creator::ERR_ERRONEUSNICKNAME(
            context.client, _serverName, context.pending_fds);
      }
		}
	
		// good to go!
		if (context.client->GetStatus() >= REGISTERED)
		{
			std::string prev_name = context.client->GetNickname();

			// send acknowledgement to user
			context.createSource = true;
			context.numericResult = -1;
			context.stringResult = new_name;
			context.client->Send(IRC_response_creator::MakeResponse(context));
			context.pending_fds->insert(context.client->GetFD());
			// TODO IRCClient::GetChannels will return std::vector<std::string> in the future
			IRCClientJoinedChannels channels = context.client->ListChannels();
			for (IRCClientJoinedChannels::iterator it = channels.begin(); it != channels.end(); it++)
			{
				// broadcast
				context.channel = it->second;
				SendMessageToChannel(kChanSendModeToExceptMe, context);
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
    if (context.params.size() != 4) {
      return IRC_response_creator::ERR_NEEDMOREPARAMS(
          context.client, _serverName, context.pending_fds, context.command);
    }
		
    if (!context.client->SetUserName(context.params[0])) {
      return IRC_response_creator::ERR_ALREADYREGISTERED(
        context.client, _serverName, context.pending_fds);
    }
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
	context.client->Send(IRC_response_creator::MakeResponse(context));

	// send RPL_YOURHOST
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname
		<< " :Your host is "<< _serverName << ", running version " << VERSION;
	context.numericResult = 2;
	context.stringResult = strstream.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));

	// send RPL_CREATED
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname
		<< " :This server was created "<< _startDate;
	context.numericResult = 3;
	context.stringResult = strstream.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));

	// send RPL_MYINFO
	// TODO set RPL_MYINFO
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname << " " << _serverName << " " << VERSION << " r oitlk";
	context.stringResult = strstream.str();
	context.numericResult = 4;
	context.client->Send(IRC_response_creator::MakeResponse(context));

	// send RPL_ISUPPORT
	// TODO set RPL_ISUPPORT
	strstream.str("");
	context.stringResult.clear();
	strstream << clientNickname << " CHANTYPES=# CASEMAPPING=ascii CHANMODES=o,kl,,it "
		<< "PREFIX=(o)@ NICKLEN=30 :are supported by this server";
	context.stringResult = strstream.str();
	context.numericResult = 5;
	context.client->Send(IRC_response_creator::MakeResponse(context));

	// send MOTD
	ActionMOTD(context);

	context.pending_fds->insert(context.client->GetFD());
}

/**************/

void IRCServer::ActionMOTD(IRCContext& context)
{
	if (context.command == MOTD)
	{
		for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++)
		{
      if (*it != _serverName) {
        return IRC_response_creator::ERR_NOSUCHSERVER(
            context.client, _serverName, context.pending_fds, *it);
      }
		}
	}
	std::string clientNickname = context.client->GetNickname();
	
	context.stringResult = clientNickname + " : -- Welcome to " + _serverName + "--";
	context.numericResult = 375;
	context.client->Send(IRC_response_creator::MakeResponse(context));
	
	context.stringResult = clientNickname + " :Mesasge of the day:";
	context.numericResult = 372;
	context.client->Send(IRC_response_creator::MakeResponse(context));
	
	context.stringResult = clientNickname + " :WeLOve42Seoul";
	context.numericResult = 372;
	context.client->Send(IRC_response_creator::MakeResponse(context));

	context.stringResult = clientNickname + " :end of MOTD";
	context.numericResult = 376;
	context.client->Send(IRC_response_creator::MakeResponse(context));

	context.pending_fds->insert(context.client->GetFD());
}
