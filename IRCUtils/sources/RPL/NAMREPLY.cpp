#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"

#include <string>
#include <sstream>
#include <deque>

#include "IRCClient.hpp"


void IRC_response_creator::RPL_NAMREPLY(IRCContext& context){
	std::deque<std::string> channel_names = context.channel->GetChannelUsersWithPrefixes();
	std::string names;
	while(!channel_names.empty())
	{
		for(unsigned int i = 0; i < channel_names.size() || i < 5; i++)
		{
			if(channel_names.empty())
				break;
			names += channel_names.front();
			channel_names.pop_front();
			if(!channel_names.empty())
				names += " ";
		}
		std::stringstream result;
		result << context.client->GetNickname()
			<< " = "<< context.channel->GetChannelInfo(kChannelName) << " :" << names;
		context.numericResult = 353;
		context.stringResult = result.str();
		context.client->Send(IRC_response_creator::MakeResponse(context));
		context.pending_fds->insert(context.client->GetFD());
		names.clear();
	}
	IRC_response_creator::RPL_ENDOFNAMES(context);
}
