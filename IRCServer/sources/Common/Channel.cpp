#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

#include <string>
#include <deque>

#include "IRCResponseCreator.hpp"

void IRCServer::SendMessageToChannel(IRCContext& context, enum ChannelSendMode target)
{
	std::string message = IRCResponseCreator::MakeResponse(context);
	ChannelUsers users = context.channel->GetChannelUsersWithPrefixes();

	for(ChannelUsers::iterator it = users.begin(); it != users.end(); it++)
	{
		std::string userNickname = *it;
		if (target == SendToOper && userNickname[0] != '@')
			continue ;
		if (userNickname[0] == '@')
			userNickname = userNickname.substr(1);
		IRCClient* userPtr = GetClient(userNickname);
		if (target != SendToAll && userPtr == context.client)
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
			SendMessageToChannel(context, SendToAllExceptMe);
		// delete name from channel
		context.channel->DelChannelUser(nickname);
		if(context.channel->GetChannelUserSize() == 0)
			DelChannel(context.channel->GetChannelInfo(kChannelName)); 
	}
	// remove from invitation list
	IRCClientChannels _invited_chans = context.client->ListInvitedChannels();
	for (IRCClientChannels::iterator it = _invited_chans.begin(); it != _invited_chans.end(); it++)
	{
		IRCChannel* channel = GetChannel(*it);
		if (channel == NULL)
			continue ;
		channel->DelInvitedUser(nickname);
	}
}
