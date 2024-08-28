#include "IRCResponseCreator.hpp"

#include <string>
#include <sstream>

#include "IRCRequestParser.hpp"
#include "IRCClient.hpp"
#include "IRCChannel.hpp"

void IRCResponseCreator::RPL_CHANNELMODEIS(IRCContext& context){
	std::stringstream result;

	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " ";
	if(context.channel->IsInChannel(context.client->GetNickname()))
		result << context.channel->GetChannelMode();
	else
		result << IRCRequestParser::SeparateParam(context.channel->GetChannelMode(), " ")[0];
	
	context.numericResult = 324;
	context.stringResult = result.str();
	context.client->Send(IRCResponseCreator::MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}
