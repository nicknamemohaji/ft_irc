#include "IRCServer.hpp"
// RPL_INVITING (341) 
//   "<client> <nick> <channel>"
//INVITE somin :#akak

void IRCServer::RPL_INVITED(IRCContext& context){
	# ifdef COMMAND
	std::cout << "INVITED_RPL start " << context.params[0] << " "<< context.client->GetNickname() << std::endl;
	# endif
	std::stringstream result;
	result.str("");
	context.stringResult.clear();
	result <<"INVITE " << context.client->GetNickname()
		<< " :"<< context.channel->GetChannelInfo(kChannelName);
	context.numericResult = -1;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));
	// IRCClient *client = GetClient(context.params[0]);
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "INVITED_RPL end" << std::endl;
	# endif
}