#include "IRCServer.hpp"
// RPL_INVITING (341) 
//   "<client> <nick> <channel>"
//INVITE somin :#akak

void IRCServer::RPL_INVITING(IRCContext& context){
	# ifdef COMMAND
	std::cout << "INVITE_RPL start " << context.params[1] << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result << context.client->GetNickname() << " "<< context.params[0]
		<< " "<< context.channel->GetChannelInfo(kChannelName);
	context.numericResult = 341;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "INVITE_RPL end" << std::endl;
	# endif
}