#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionPING(IRCContext& context)
{
	// TODO PING
	context.stringResult = context.client->GetNickname() + ":" + _serverName;
	context.client->Send(MakeResponse(context));
	context.FDsPendingWrite.insert(context.client->GetFD());
}
