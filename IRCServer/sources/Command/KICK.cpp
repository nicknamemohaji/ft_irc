#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>
#include <queue>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionKICK(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "*KICK command start*" << std::endl;
	std::cout << "context.params.size = " << context.params.size() << std::endl;

	for(unsigned long i = 0; i < context.params.size(); i++)
		std::cout << context.params[i] << std::endl;
	# endif

	IRCChannel *channel;
	std::string result;
	if(context.params.size() <= 1)
		throw IRCError::MissingParams(); // 461

	std::string channel_name = context.params[0];
	if(GetChannel(channel_name) == NULL)
		return;
	channel = GetChannel(AddPrefixToChannelName(channel_name));
	context.channel = channel;
	if(!channel){
		context.stringResult = channel_name; 
		throw IRCError::NoSuchChannel(); //403
	}
	std::string user_name = context.client->GetNickname();
	if(!channel->IsInChannel(user_name))
		throw IRCError::NotOnChannel(); //ERR_NOTONCHANNEL 442	
	if(!channel->IsUserAuthorized(user_name, kOperator))
		throw IRCError::ChangeNoPrivesneed(); //CHANOPRIVSNEEDED 482
	std::vector<std::string> target_name = ParserSep(context.params[1] , ",");
	if(!IsUserInList(target_name[0]))
		throw IRCError::ChangeNoPrivesneed(); // ERR_NOSUCHNICK 401
	if(!channel->IsInChannel(target_name[0]))
		throw IRCError::UserNotInChannel(); //ERR_USERNOTINCHANNEL 441
	// if(channel->IsUserAuthorized(target_name[0]], kOperator))
	// 	channel->SetUserAuthorization(target_name[0]], kNormal);
	result += target_name[0] + " ";
	channel->DelChannelUser(target_name[0]);
	if(context.params.size() == 2)
		result += ":" + target_name[0];
	else {
		if(context.params[2][0] == ':')
			for(unsigned int i = 2; i < context.params.size(); i++)
				result += context.params[i] + " ";
		else {
			result += ":" + context.params[2];
		}
	}
	# ifdef COMMAND
	std::cout << "result = " << result << std::endl;
	# endif
	context.numericResult = -1;
	context.createSource = true;
	context.stringResult = " KICK " + context.channel->GetChannelInfo(kChannelName) + " :" + result;
	SendMessageToChannel(context, SendToAll);
}