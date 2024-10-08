#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>
#include <queue>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCRequestParser.hpp"
#include "IRCTypes.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"
#include "IRCUtils/includes/IRCResponseCreator.hpp"

void IRCServer::ActionKICK(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "*KICK command start*" << std::endl;
	std::cout << "context.params.size = " << context.params.size() << std::endl;

	for(unsigned long i = 0; i < context.params.size(); i++)
		std::cout << context.params[i] << std::endl;
	# endif

  if (context.params.size() <= 1) {
    return IRC_response_creator::ERR_NEEDMOREPARAMS(
      context.client, server_name_, context.pending_fds, context.command);
  }

	std::string channel_name = context.params[0];
	IRCChannel *channel = GetChannel(IRC_request_parser::AddChanPrefixToParam(channel_name));
	if(!channel){
		context.stringResult = channel_name; 
		IRC_response_creator::ErrorSender(context, 403);
		return;
	}
	context.channel = channel;
	std::string user_name = context.client->GetNickname();
	if(!channel->IsInChannel(user_name)) {
		IRC_response_creator::ErrorSender(context, 442);
		return;
	}
	if(!channel->IsUserAuthorized(user_name, kOperator)) {
		IRC_response_creator::ErrorSender(context, 482);
		return;
	}
	IRCParams target_name = IRC_request_parser::SeparateParam(context.params[1] , ",");
	for(unsigned int i = 0; i < target_name.size(); i++) {
		if(!IsUserInList(target_name[i])) {
			context.stringResult = target_name[i];
			IRC_response_creator::ErrorSender(context, 401);
			return;
		}
		if(!channel->IsInChannel(target_name[i])) {
			context.stringResult = target_name[i];
   			IRC_response_creator::ErrorSender(context, 441);
			return;
		}
		std::string kick_result;
		kick_result = target_name[i];
		std::string add_result;
		if((context.params.size() == 2) || (context.params.size() > 2 && context.params[2].size() == 0))
			add_result = target_name[i];
		else {
			if(context.params[2][0] == ':') {
				for(unsigned int i = 2; i < context.params.size(); i++)
					add_result += context.params[i] + " ";
				add_result.erase(0,1);
			}
			else
				add_result = context.params[2];
		}
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = context.channel->GetChannelInfo(kChannelName) + " " + kick_result + " :" + add_result;
		SendMessageToChannel(kChanSendModeToAll, context);
		channel->DelChannelUser(target_name[0]);
		IRCClient *target_user = GetClient(target_name[i]);
		target_user->DelChannel(channel_name);
		if(context.channel->GetChannelUserSize() == 0)
			DelChannel(channel_name);
	}
}