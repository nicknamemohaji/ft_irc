#include "IRCServer.hpp"
#include "IRCChannel.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>
#include <queue>

#include "IRCTypes.hpp"
#include "IRCRequestParser.hpp"
#include "IRCUtils/includes/IRCResponseCreator.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionMODE(IRCContext& context)
{	
	IRCChannel *channel;
  if (context.params.size() <= 0) {
    return IRC_response_creator::ERR_NEEDMOREPARAMS(
        context.client, server_name_, context.pending_fds, context.command);
  } else {
		std::string channel_name = context.params[0];
		channel = this->GetChannel(IRC_request_parser::AddChanPrefixToParam(channel_name));
		context.channel = channel;
		if(!channel){
			context.stringResult = channel_name; 
			IRC_response_creator::ErrorSender(context, 403);
			return;
		}
		std::string user_name = context.client->GetNickname();
		if(context.params.size() == 1) {
			IRC_response_creator::RPL_CHANNELMODEIS(context); //CHANNELMODEIS 324 
			IRC_response_creator::RPL_CREATIONTIME(context); //CREATIONTIME 329
			return;
		}
		if(!channel->IsUserAuthorized(user_name, kOperator)) {
			IRC_response_creator::ErrorSender(context, 482);
			return;
		}
		std::string mode_str = "-+itlko";
		for(unsigned int i = 0; i < context.params[1].size(); i++)
			if(mode_str.find(context.params[1][i]) == std::string::npos) {
				context.stringResult = context.params[1][i];
				IRC_response_creator::ErrorSender(context, 472);
				return;
			}
	}

	std::string mode_result;
	std::queue<std::string> add_result;
	unsigned int idx = 2;
	int flag = true;
	for(unsigned int i = 0; i < context.params[1].size(); i++) {
		if(context.params[1][i] == '-') {
			if(mode_result.size() && (mode_result[mode_result.size()-1] == '-' || mode_result[mode_result.size()-1] == '+'))
				mode_result.erase(mode_result.size()-1);
			mode_result += "-";
			flag = false;
		}
		else if(context.params[1][i] == '+') {
			if(mode_result.size() && (mode_result[mode_result.size()-1] == '-' || mode_result[mode_result.size()-1] == '+'))
				mode_result.erase(mode_result.size()-1);
			mode_result += "+";
			flag = true;
		}
		else {
			if(context.params[1][i] == 'i') {
				if((!channel->CheckChannelMode(kInvite) && flag) || (channel->CheckChannelMode(kInvite) && !flag)){
					channel->SetChannelMode(kInvite, flag);
					mode_result += "i";
				}
			}
			else if(context.params[1][i] == 't') {
				if((!channel->CheckChannelMode(kTopic) && flag) || (channel->CheckChannelMode(kTopic) && !flag)) {
					channel->SetChannelMode(kTopic, flag);
					mode_result += "t";
				}
			}
			else if(context.params[1][i] == 'k') {
				if(idx >= context.params.size())
					continue;
				std::string pwd = context.params[idx++];
				if(pwd == "x")
					continue;
				if(flag) {
					channel->SetChannelMode(kPassword, flag);
					channel->SetChannelInfo(kChannelPassword, pwd);
					mode_result += "k";
					add_result.push(pwd);
				}
				else if(channel->CheckChannelMode(kPassword) && !flag) {
					channel->SetChannelMode(kPassword, flag);
					mode_result += "k";
					add_result.push("*");
				}
			}
			else if(context.params[1][i] == 'o') {
				if(idx >= context.params.size())
					continue;
				std::string target_name = context.params[idx++];
				if(!IsUserInList(target_name)) {
					context.stringResult = target_name;
					IRC_response_creator::ErrorSender(context,401);
					continue;
				}
				if(!channel->IsInChannel(target_name)){
					context.stringResult = target_name;
					IRC_response_creator::ErrorSender(context,441);
					continue;
				}
				mode_result += "o";
				add_result.push(target_name);
				if(flag)
					channel->SetUserAuthorization(target_name, kOperator);
				else
					channel->SetUserAuthorization(target_name, kNormal);
			}
			else if(context.params[1][i] == 'l') {
				if(channel->CheckChannelMode(kLimit) && !flag) {
					channel->SetChannelMode(kLimit, flag);
					mode_result += "l";
				}
				else if(flag) {
					if(idx >= context.params.size())
						continue;
					std::string limit_str = context.params[idx++];
					unsigned int limit = strtod(limit_str.c_str(), nullptr);
					if(limit < 1)
						continue;
					if(limit > kMaxChannelUsers)
						limit = kMaxChannelUsers;
					channel->SetChannelMode(kLimit, flag);
					channel->SetChannelInfo(kChannelUserLimit, std::to_string(limit));
					mode_result += "l";
					add_result.push(std::to_string(limit));
				}
			}
		}
	}
	if(mode_result.size() && (mode_result[mode_result.size()-1] == '-' || mode_result[mode_result.size()-1] == '+'))
		mode_result.erase(mode_result.size()-1);
	if(mode_result.size() > 1) {
		while(!add_result.empty()) {
			mode_result += " " + add_result.front();
			add_result.pop();
		}
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = context.channel->GetChannelInfo(kChannelName) + " " + mode_result;
		SendMessageToChannel(kChanSendModeToAll, context);
	}
}