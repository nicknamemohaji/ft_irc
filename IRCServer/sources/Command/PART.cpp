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

void IRCServer::ErrorSender(IRCContext context, unsigned int errornum){
	# ifdef COMMAND
	std::cout << "Error sender process "  << errornum << std::endl;
	# endif
	context.numericResult = errornum;
	context.client->Send(this->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
}
void sendPartMsg(std::deque<std::string> user_in_channel, IRCContext context, IRCServer& server, std::string& reason)
{	
	std::stringstream result;
	IRCClient *user_to_msg;
	std::string user_name = context.client->GetNickname();
	# ifdef COMMAND
	std::cout << "PART msg to channel size = "  << user_in_channel.size() << std::endl;
	# endif
	for(unsigned int i = 0; i < user_in_channel.size(); ++i){
		# ifdef COMMAND
		std::cout << "PART msg to channel user = "  << user_in_channel[i] << std::endl;
		# endif
		std::string ret;
		result.str("");
		// result <<":"<< user_name << "!" << user_name << "@ft_irc.com"
		result <<":"<< user_name
		<< " PART " << context.channel->GetChannelInfo(kChannelName);
		if(reason.size() > 1)
			result << " :" << reason;
		result << "\r\n";
		ret = result.str();
		user_to_msg = server.GetClient(user_in_channel[i]);
		if(!user_to_msg)
			continue;
	# ifdef COMMAND
		// std::cout << "Sucesse sned PART msg to channel, to " << user_to_msg->GetNickname()  << "the fd is" << user_to_msg->GetFD() << std::endl;
		std::cout << "msg" << std::endl;
		std::cout << ret << std::endl;
	# endif	
		context.client = user_to_msg;
		context.client->Send(ret);
		context.FDsPendingWrite.insert(context.client->GetFD());
	}
	# ifdef COMMAND
		std::cout << "PART msg to channel end "  << std::endl;
	# endif
}
void IRCServer::ActionPART(IRCContext& context){
		//  param size =1 channel left;
			std::cout << " PRAT param size = " << context.params.size() <<std::endl;
		if(!(context.params.size() > 0 && context.params.size() < 3)){
			ErrorSender(context, 461);
			return;
		}
		# ifdef COMMAND
			std::cout << "param 0 =" << context.params[0] <<std::endl;
			std::cout << "param 1 = " << context.params[1] <<std::endl;
		# endif
		std::string reason;
		if(context.params.size() == 2)
			reason = context.params[1];
		std::vector<std::string>channel_name_arry =  ParserSep(context.params[0],",");
		#ifdef COMMAND
		std::cout<< "result of paser channel name = " << std::endl;
		for(unsigned int i = 0; i < channel_name_arry.size(); i++){
			std::cout << "channel name "<< i <<" "<< channel_name_arry[i] << std::endl;
		}
		#endif	
		for(unsigned int i = 0; i < channel_name_arry.size(); i++){
			context.channel = GetChannel(AddPrefixToChannelName(channel_name_arry[i]));
			std::string channel_name = AddPrefixToChannelName(channel_name_arry[i]);
			if(!context.channel){
				#ifdef COMMAND
				//채널없음
				std::cout<< "channel error no channel" << channel_name << std::endl;
				#endif	
				context.stringResult = channel_name;
				ErrorSender(context,403);
				continue;
			}
			if(!context.channel->IsInChannel(context.client->GetNickname())){
				//채널에 유저 없음
				ErrorSender(context,442);
				continue;
			}
			sendPartMsg(context.channel->GetMemberNames(),context,*this,reason);
			context.channel->DelChannelUser(context.client->GetNickname());
			if(context.channel->GetChannelUserSize() == 0)
				this->DelChannel(channel_name);
			context.client->DelChannel(channel_name);
		}
}