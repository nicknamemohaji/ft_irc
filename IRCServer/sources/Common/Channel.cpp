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

void IRCServer::RemoveClientFromChannel(IRCContext& context)
{
	std::string nickname = context.client->GetNickname();
	context.numericResult = -1;
	context.createSource = true;

	// remove from channel users
	IRCClientJoinedChannels channels = context.client->ListChannels();
	for (IRCClientJoinedChannels::iterator it = channels.begin(); it != channels.end(); it++)
	{
		context.channel = it->second;
		// broadcast
		if (context.stringResult.size() != 0)
			SendMessageToChannel(context, false);
		// delete name from channel
		context.channel->DelChannelUser(nickname);
		if(context.channel->GetChannelUserSize() == 0)
			DelChannel(context.channel->GetChannelInfo(kChannelName)); 
	}
	// remove from invitation list
	IRCClientChannels invited_chans = context.client->ListInvitedChannels();
	for (IRCClientChannels::iterator it = invited_chans.begin(); it != invited_chans.end(); it++)
	{
		IRCChannel* channel = GetChannel(*it);
		if (channel == NULL)
			continue ;
		channel->DelInvitedUser(nickname);
	}
}
