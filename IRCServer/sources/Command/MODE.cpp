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

void sendModeMsg(std::deque<std::string> user_in_channel, IRCContext context, IRCServer& server, std::string& mode_result)
{	
	std::stringstream result;
	IRCClient *user_to_msg;
	std::string user_name = context.client->GetNickname();
	# ifdef COMMAND
	std::cout << "MODE msg to channel size = "  << user_in_channel.size() << std::endl;
	# endif
	for(unsigned int i = 0; i < user_in_channel.size(); ++i){
		# ifdef COMMAND
		std::cout << "MODE msg to channel user = "  << user_in_channel[i] << std::endl;
		# endif
		std::string ret;
		result.str("");
		// result <<":"<< user_name << "!" << user_name << "@ft_irc.com"
		result <<":"<< user_name
		<< " MODE " << context.channel->GetChannelInfo(kChannelName);
		if(mode_result.size() > 1)
			result << " :" << mode_result;
		result << "\r\n";
		ret = result.str();
		user_to_msg = server.GetClient(user_in_channel[i]);
		if(!user_to_msg)
			continue;
	# ifdef COMMAND
		// std::cout << "Sucesse sned MODE msg to channel, to " << user_to_msg->GetNickname()  << "the fd is" << user_to_msg->GetFD() << std::endl;
		std::cout << "msg" << std::endl;
		std::cout << ret << std::endl;
	# endif	
		context.client = user_to_msg;
		context.client->Send(ret);
		context.FDsPendingWrite.insert(context.client->GetFD());
	}
	# ifdef COMMAND
		std::cout << "MODE msg to channel end "  << std::endl;
	# endif
}

void IRCServer::ActionMODE(IRCContext& context)
{
	# ifdef COMMAND
	std::cout << "*MODE command start*" << std::endl;
	std::cout << "context.params.size = " << context.params.size() << std::endl;

	for(unsigned long i = 0; i < context.params.size(); i++)
		std::cout << context.params[i] << std::endl;
	# endif
	
	IRCChannel *channel;
	if(context.params.size() <= 0)
		throw IRCError::MissingParams(); // 461
	else {
		std::string channel_name = context.params[0];
		if(GetChannel(channel_name) == NULL)
			return;
		channel = this->GetChannel(AddPrefixToChannelName(channel_name));
		context.channel = channel;
		if(!channel){
			context.stringResult = channel_name; 
			throw IRCError::NoSuchChannel(); //403
		}
		if(context.params.size() == 1) {
			//this->RPL_CHANNELMODEIS(context); //CHANNELMODEIS 324 (채널 내부인/외부인 구분 필요)
			this->RPL_CREATIONTIME(context); //CREATIONTIME 329
			return;
		}
		std::string user_name = context.client->GetNickname();
		if(!channel->IsUserAuthorized(user_name, kOperator))
			throw IRCError::ChangeNoPrivesneed(); //CHANOPRIVSNEEDED 482
		std::string mode_str = "-+itlko";
		for(unsigned int i = 0; i < context.params[1].size(); i++)
			if(mode_str.find(context.params[1][i]) == std::string::npos)
				throw IRCError::UnKnownModeChar(); //UNKNOWNMODE 472
	}
	
	std::string mode_result;
	std::queue<std::string> add_result;
	unsigned int idx = 2;
	int flag = true;
	for(unsigned int i = 0; i < context.params[1].size(); i++) {
		if(context.params[1][i] == '-') {
			mode_result += "-";
			flag = false;
		}
		else if(context.params[1][i] == '+') {
			mode_result += "+";
			flag = true;
		}
		else {
			if(context.params[1][i] == 'i') {
				if((!channel->CheckChannelMode(kInvite) && flag) || (channel->CheckChannelMode(kInvite) && !flag)){
					channel->SetInvite(flag);
					mode_result += "i";
				}
			}
			else if(context.params[1][i] == 't') {
				if((!channel->CheckChannelMode(kTopic) && flag) || (channel->CheckChannelMode(kTopic) && !flag)) {
					channel->SetTopic(flag);
					mode_result += "t";
				}
			}
			else if(context.params[1][i] == 'k') {
				if(idx >= context.params.size())
					continue;
				std::string pwd = context.params[idx++];
				if(!channel->CheckChannelMode(kPassword) && flag) {
					if(pwd == "x")
						continue;
					channel->SetPassword(pwd);
					mode_result += "k";
					add_result.push(pwd);
				}
				else if(channel->CheckChannelMode(kPassword) && !flag) {
					channel->SetPassword("");
					mode_result += "k";
					add_result.push("*");
				}
			}
			else if(context.params[1][i] == 'o') {
				if(idx >= context.params.size())
					continue;
				std::string target_name = context.params[idx++];
				if(!IsUserInList(target_name)) {
					ErrorSender(context,401);
					continue;
				}
				if(!channel->IsInChannel(target_name)){
					ErrorSender(context,441);
					continue;
				}
				mode_result += "o";
				add_result.push(target_name);
				if(flag)
					channel->SetUserAuthorization(target_name, kOperator);//operator권한 주는함수?
				else
					channel->SetUserAuthorization(target_name, kNormal);
			}
			else if(context.params[1][i] == 'l') {
				if(channel->CheckChannelMode(kLimit) && !flag) {
					channel->SetLimit(-1);
					mode_result += "l";
				}
				else if(!channel->CheckChannelMode(kLimit) && flag) {
					if(idx >= context.params.size())
						continue;
					std::string limit_str = context.params[idx++];
					unsigned int limit = strtod(limit_str.c_str(), nullptr);
					if(limit < 1 || limit > channel->channel_limit_)
						continue;
					channel->SetLimit(limit);
					mode_result += "l";
					add_result.push(std::to_string(limit));
				}
			}
		}
	}
	std::string sign= "-+";
	for(int i = 0; i < mode_result.size()-1; i++) {
		if((sign.find(mode_result[i]) != std::string::npos) && (sign.find(mode_result[i+1]) != std::string::npos)){}
	}
	while(!add_result.empty()) {
		mode_result += " " + add_result.front();
		add_result.pop();
	}
	sendModeMsg(channel->GetMemberNames(),context,*this,mode_result);
}