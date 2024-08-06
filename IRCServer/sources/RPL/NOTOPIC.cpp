#include "IRCRpl.hpp"

void IRCRpl::RPL_NOTOPIC(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_NOTOPIC start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :No topic is set";
	context.numericResult = 331;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));	
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_NOTOPIC end" << std::endl;
	# endif
}