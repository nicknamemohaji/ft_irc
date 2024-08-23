#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"

#include <string>
#include <sstream>
#include <deque>

void IRCServer::ActionQUIT(IRCContext& context)
{
	IRCClient* client = context.client;
	std::string clientName = client->GetNickname();

	// prepare response
	context.createSource = true;
	context.numericResult = -1;
	std::stringstream ss;
	ss << "QUIT :";
	for (std::deque<std::string>::iterator it = context.params.begin(); it != context.params.end(); it++)
		ss << *it << " ";
	context.stringResult = ss.str();
	
	// delete from channels
	RemoveClientFromChannel(context);
	client->Send(MakeResponse(context));
	// TODO remove from invitelist after merge

	// acknoledgement to client
	context.stringResult = "ERROR: Quit connection";
	client->Send(MakeResponse(context));
	context.FDsPendingWrite.insert(client->GetFD());
	client->SetStatus(PENDING_QUIT);
}
