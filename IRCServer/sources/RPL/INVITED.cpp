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
	context.createSource = true;
	result <<"INVITE " << context.client->GetNickname()
		<< " "<< context.channel->GetChannelInfo(kChannelName);
	context.numericResult = -1;
	context.stringResult = result.str();
	context.client->Send(this->MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
	# ifdef COMMAND
	std::cout << "INVITED_RPL end" << std::endl;
	# endif
}

// :default_user!~root@121.135.181.42 INVITE somin :#bb
// sabyun!~root@127.0.0.1 INVITE sabyun :#aa