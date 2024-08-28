#include "IRCResponseCreator.hpp"

#include <string>
#include <sstream>

#include "IRCContext.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"

void IRCResponseCreator::RPL_ENDOFNAMES(IRCContext& context){
	std::stringstream result;
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :End of /NAMES list";
	
	context.numericResult = 366;
	context.stringResult = result.str();
	context.client->Send(IRCResponseCreator::MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}