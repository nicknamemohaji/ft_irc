#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

#include <string>
#include <deque>

void IRCServer::SendMessageToChannel(IRCContext& context, bool sendAlso)
{
	std::string message = MakeResponse(context);
	ChannelUsers users = context.channel->GetMemberNames();

	for(ChannelUsers::iterator it = users.begin(); it != users.end(); it++)
	{
		// TODO handle error
		IRCClient* userPtr = GetClient(*it);
		if (!sendAlso && userPtr == context.client)
			continue ;
		userPtr->Send(message);
		context.FDsPendingWrite.insert(userPtr->GetFD());
	}
}
