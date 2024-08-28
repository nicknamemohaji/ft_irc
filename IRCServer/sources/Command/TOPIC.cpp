#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCRequestParser.hpp"
#include "IRCTypes.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionTOPIC(IRCContext &context)
{
	if(!(context.params.size() == 1 || context.params.size() == 2)){
		ErrorSender(context, 461);
		return;
	}
#ifdef COMMAND
	std::cout << "param size = " << context.params.size() << std::endl;
	for (unsigned int i = 0; i < context.params.size(); i++)
	{
		std::cout << "param data =  " << i << ": " << context.params[i] << std::endl;
	}
#endif
	IRCChannel *channel = this->GetChannel(IRCRequestParser::AddChanPrefixToParam(context.params[0]));
	context.channel = channel;
	if (!channel)
	{
		context.stringResult = IRCRequestParser::AddChanPrefixToParam(context.params[0]);
		ErrorSender(context, 403); // no such channel
		return;
	}
	if(!channel->IsInChannel(context.client->GetNickname())){
		ErrorSender(context, 442); // you are not in channel
		return;
	}
	if(channel->CheckChannelMode(kTopic) && !channel->IsUserAuthorized(context.client->GetNickname(), kOperator)){
		//channel mode t and nor operater
		ErrorSender(context, 482);
		return;
	}
	if(context.params.size() == 2){
		channel->SetChannelInfo(kTopicInfo,context.params[1]);
		channel->SetChannelInfo(kTopicEditUser,context.client->GetNickname());
		channel->SetChannelInfo(kTopicEditTime, channel->itostr(std::time(NULL)));
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = " TOPIC " + context.channel->GetChannelInfo(kChannelName) + " :" + context.channel->GetChannelInfo(kTopicInfo);
		SendMessageToChannel(context, SendToAll);
	}
	else{
	if (context.channel->GetChannelInfo(kTopicInfo).size() == 0)
		RPL_NOTOPIC(context);
	else
		RPL_TOPIC(context);
	RPL_TOPICWHOTIME(context);
	}
}