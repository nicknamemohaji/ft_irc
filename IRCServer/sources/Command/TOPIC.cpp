#include "IRCServer.hpp"
#include "IRCChannel.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCTypes.hpp"
#include "IRCRequestParser.hpp"
#include "IRCResponseCreator.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionTOPIC(IRCContext &context)
{
	if(!(context.params.size() == 1 || context.params.size() == 2)){
		IRCResponseCreator::ErrorSender(context, 461);
		return;
	}

	IRCChannel *channel = this->GetChannel(IRC_request_parser::AddChanPrefixToParam(context.params[0]));
	context.channel = channel;
	if (!channel)
	{
		context.stringResult = IRC_request_parser::AddChanPrefixToParam(context.params[0]);
		IRCResponseCreator::ErrorSender(context, 403); // no such channel
		return;
	}
	if(!channel->IsInChannel(context.client->GetNickname())){
		IRCResponseCreator::ErrorSender(context, 442); // you are not in channel
		return;
	}
	if(context.params.size() == 2){
		if(channel->CheckChannelMode(kTopic) && !channel->IsUserAuthorized(context.client->GetNickname(), kOperator)){
			//channel mode t and nor operater
			IRCResponseCreator::ErrorSender(context, 482);
			return;
		}
		channel->SetChannelInfo(kTopicInfo,context.params[1]);
		channel->SetChannelInfo(kTopicEditUser,context.client->GetNickname());
		channel->SetChannelInfo(kTopicEditTime, channel->itostr(std::time(NULL)));
		context.numericResult = -1;
		context.createSource = true;
		context.stringResult = " TOPIC " + context.channel->GetChannelInfo(kChannelName) + " :" + context.channel->GetChannelInfo(kTopicInfo);
		SendMessageToChannel(kChanSendModeToAll, context);
	}
	else
	{
		if (context.channel->GetChannelInfo(kTopicInfo).size() == 0)
			IRCResponseCreator::RPL_NOTOPIC(context);
		else
			IRCResponseCreator::RPL_TOPIC(context);
		IRCResponseCreator::RPL_TOPICWHOTIME(context);
	}
}