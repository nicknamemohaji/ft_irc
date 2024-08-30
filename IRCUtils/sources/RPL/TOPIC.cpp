#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"

#include <string>
#include <sstream>

#include "IRCClient.hpp"

void IRC_response_creator::RPL_TOPIC(IRCContext& context){
	std::stringstream result;
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :" << context.channel->GetChannelInfo(kTopicInfo);
	
	context.numericResult = 332;
	context.stringResult = result.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));	
	context.pending_fds->insert(context.client->GetFD());
}
