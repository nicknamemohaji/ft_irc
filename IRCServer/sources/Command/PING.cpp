#include "IRCServer.hpp"
#include "IRCContext.hpp"

#include <string>
#include <sstream>

#include "IRCResponseCreator.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionPING(IRCContext& context)
{
	if (context.params.size() != 1)
		throw IRCError::MissingParams();
	
	context.createSource = false;
	std::stringstream ss;
	ss << "PONG " << _serverName << " :" << context.params[0];
	context.stringResult = ss.str();
	context.client->Send(IRCResponseCreator::MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}
