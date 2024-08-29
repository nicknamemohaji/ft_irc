#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"

#include <string>
#include <sstream>

#include "IRCClient.hpp"

void IRCResponseCreator::RPL_TOPICWHOTIME(IRCContext& context){
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.channel->GetChannelInfo(kTopicEditUser) << " " << context.channel->GetChannelInfo(kTopicEditTime);
	context.numericResult = 333;
	context.stringResult = result.str();
	context.client->Send(IRCResponseCreator::MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
}
