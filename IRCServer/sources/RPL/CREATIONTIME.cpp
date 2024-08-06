#include "IRCRpl.hpp"
//RPL_CREATIONTIME (329)
// "<client> <channel> <creationtime>"

void IRCRpl::RPL_CREATIONTIME(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_CREATIONTIME start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << context.channel->GetChannelInfo(kChannelDate);
	context.numericResult = 329;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_CREATIONTIME end" << std::endl;
	# endif
}