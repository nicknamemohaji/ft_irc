#include "IRCServer.hpp"

void IRCServer::RPL_TOPIC(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_TOPIC start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :" << context.channel->GetChannelInfo(kTopicInfo);
	context.numericResult = 332;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_TOPIC end" << std::endl;
	# endif
}