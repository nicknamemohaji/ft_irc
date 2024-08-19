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
void sendTopicMsg(std::deque<std::string> user_in_channel, IRCContext context, IRCServer& server)
{	
	std::stringstream result;
	IRCClient *user_to_msg;
	std::string user_name = context.client->GetNickname();
	# ifdef COMMAND
	std::cout << "TOPIC msg to channel size = "  << user_in_channel.size() << std::endl;
	# endif
	for(unsigned int i = 0; i < user_in_channel.size(); ++i){
		# ifdef COMMAND
		std::cout << "TOPIC msg to channel user = "  << user_in_channel[i] << std::endl;
		# endif
		std::string ret;
		result.str("");
		// result <<":"<< user_name << "!" << user_name << "@ft_irc.com"
		result << " TOPIC :" << context.channel->GetChannelInfo(kChannelName) << ": new topic\r\n";
		ret = result.str();
		user_to_msg = server.GetClient(user_in_channel[i]);
		if(!user_to_msg)
			continue;
	# ifdef COMMAND
		// std::cout << "Sucesse sned TOPIC msg to channel, to " << user_to_msg->GetNickname()  << "the fd is" << user_to_msg->GetFD() << std::endl;
		std::cout << "msg" << std::endl;
		std::cout << ret << std::endl;
	# endif	
		context.client = user_to_msg;
		context.client->Send(ret);
		context.FDsPendingWrite.insert(context.client->GetFD());
		if(context.channel->GetChannelInfo(kTopicInfo).size() == 0)
			server.RPL_NOTOPIC(context);
		else
			server.RPL_TOPIC(context);
		server.RPL_TOPICWHOTIME(context);
	}
	# ifdef COMMAND
		std::cout << "TOPIC msg to channel end "  << std::endl;
	# endif
}

void IRCServer::ActionTOPIC(IRCContext &context)
{
#ifdef COMMAND
	std::cout << "param size = " << context.params.size() << std::endl;
	for (unsigned int i = 0; i < context.params.size(); i++)
	{
		std::cout << "param data =  " << i << ": " << context.params[i] << std::endl;
	}
#endif
	IRCChannel *channel = this->GetChannel(AddPrefixToChannelName(context.params[0]));
	context.channel = channel;
	if (!channel)
	{
		context.stringResult = AddPrefixToChannelName(context.params[0]);
		ErrorSender(context, 403);
		return;
	}
	if(!channel->IsInChannel(context.client->GetNickname())){
		ErrorSender(context, 442);
		return;
	}
	/*
	channel mode t check
	do here.
	*/
	if(context.params.size() > 1){
		channel->SetChannelInfo(kTopicInfo,context.params[1]);
		channel->SetChannelInfo(kTopicEditUser,context.client->GetNickname());
		channel->SetChannelInfo(kTopicEditTime,channel->itostr(std::time(NULL)));
	}
	// if(channel->GetChannelInfo(kTopicInfo).size() == 0){
	// 	this->RPL_NOTOPIC(context);
	// 	return;
	// }
	sendTopicMsg(channel->GetMemberNames(),context,*this);
}