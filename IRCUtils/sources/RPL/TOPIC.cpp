#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"

#include <string>
#include <sstream>

#include "IRCClient.hpp"

void IRCResponseCreator::RPL_TOPIC(IRCContext& context){
	std::stringstream result;
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :" << context.channel->GetChannelInfo(kTopicInfo);
	
	context.numericResult = 332;
	context.stringResult = result.str();
	context.client->Send(IRCResponseCreator::MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
}
