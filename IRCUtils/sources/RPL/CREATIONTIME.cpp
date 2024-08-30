#include "IRCResponseCreator.hpp"

#include <string>
#include <sstream>

#include <IRCContext.hpp>
#include "IRCClient.hpp"
#include "IRCChannel.hpp"

//RPL_CREATIONTIME (329)
// "<client> <channel> <creationtime>"

void IRC_response_creator::RPL_CREATIONTIME(IRCContext& context){
	std::stringstream result;
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.channel->GetChannelInfo(kChannelDate);
	
	context.numericResult = 329;
	context.stringResult = result.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}
