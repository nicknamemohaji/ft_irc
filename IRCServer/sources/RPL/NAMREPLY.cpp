#include "IRCRpl.hpp"

void IRCRpl::RPL_NAMREPLY(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_NAMREPLY start" << std::endl;
	# endif
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
		result.str("");
		context.stringResult.clear();
		result << context.client->GetNickname()
			<< " = "<< context.channel->GetChannelInfo(kChannelName) << " :" << names;
		context.numericResult = 353;
		context.stringResult = result.str();
		context.client->Send(context.server->MakeResponse(context));
		context.FDsPendingWrite.insert(context.client->GetFD());
		names.clear();
	}
	# ifdef COMMAND
	std::cout << "RPL_NAMREPLY end" << std::endl;
	# endif
	IRCRpl::RPL_ENDOFNAMES(context);
}