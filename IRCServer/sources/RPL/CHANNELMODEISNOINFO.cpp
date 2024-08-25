#include "IRCServer.hpp"

void IRCServer::RPL_CHANNELMODEISNOINFO(IRCContext& context){
	# ifdef COMMAND
	std::cout << "RPL_CHANNELMODEISNOINFO start" << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	
	result << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName) << " " << ParserSep(context.channel->GetChannelMode(), " ")[0];
	context.numericResult = 324;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "RPL_CHANNELMODEISNOINFO end" << std::endl;
	# endif
}