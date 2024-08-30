#include "IRCResponseCreator.hpp"

#include <string>
#include <sstream>

#include "IRCRequestParser.hpp"
#include "IRCClient.hpp"
#include "IRCChannel.hpp"

void IRC_response_creator::RPL_CHANNELMODEIS(IRCContext& context){
	std::stringstream result;

	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " ";
	if(context.channel->IsInChannel(context.client->GetNickname()))
		result << context.channel->GetChannelMode();
	else
		result << IRC_request_parser::SeparateParam(context.channel->GetChannelMode(), " ")[0];
	
	context.numericResult = 324;
	context.stringResult = result.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}
