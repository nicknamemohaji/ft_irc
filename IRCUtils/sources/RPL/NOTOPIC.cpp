#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"

#include <string>
#include <sstream>

#include "IRCClient.hpp"

void IRC_response_creator::RPL_NOTOPIC(IRCContext& context){
	std::stringstream result;
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :No topic is set";
	
	context.numericResult = 333;
	context.stringResult = result.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));	
	context.pending_fds->insert(context.client->GetFD());
}