#include "IRCServer.hpp"

void IRCServer::RPL_CHANNELMODEIS(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_CHANNELMODEIS start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.channel->GetChannelMode();
	context.numericResult = 324;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_CHANNELMODEIS end" << std::endl;
	# endif
}