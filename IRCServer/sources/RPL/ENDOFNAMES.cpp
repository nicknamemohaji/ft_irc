#include "IRCRpl.hpp"

void IRCRpl::RPL_ENDOFNAMES(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_ENDOFNAMES start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " :End of /NAMES list";
	context.numericResult = 366;
	context.stringResult = result.str();
	context.client->Send(context.server->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_ENDOFNAMES end" << std::endl;
	# endif
}