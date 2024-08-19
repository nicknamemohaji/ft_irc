#include "IRCServer.hpp"

void IRCServer::RPL_TOPICWHOTIME(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_TOPICWHOTIME start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.channel->GetChannelInfo(kTopicEditUser) << " " << context.channel->GetChannelInfo(kTopicEditTime);
	context.numericResult = 333;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_TOPICWHOTIME end" << std::endl;
	# endif
}