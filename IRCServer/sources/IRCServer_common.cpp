#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

#include <string>
#include <deque>

#include "IRCResponseCreator.hpp"

/*
IRCServer_common.cpp
- void IRCServer::SendMessageToChannel
- void IRCServer::RmClientFromChanJoined
- void IRCServer::RmClientFromChanInvited
*/

/**
 * @brief 채널에 등록된 사용자 모두에게 메시지를 전달합니다. target을 통해 전달 대상을 설정할 수 있습니다.
 * 
 * @param[in] target 전달 대상을 enum ChannelSendMode를 통해 설정
 * @param[in,out] context 전달할 메시지와 채널 정보를 담고 있는 IRCContext
*/
void IRCServer::SendMessageToChannel(enum ChannelSendMode target, IRCContext& context)
{
	std::string _message = IRCResponseCreator::MakeResponse(context);
	ChannelUsers _users = context.channel->GetChannelUsersWithPrefixes();

	for(ChannelUsers::iterator _it = _users.begin(); _it != _users.end(); _it++)
	{
		std::string _user_name = *_it;
		// check membership prefix
		if (target == kChanSendModeToOper && _user_name[0] != '@')
			continue ;
		if (_user_name[0] == '@')
			_user_name = _user_name.substr(1);
		IRCClient* _userPtr = GetClient(_user_name);
		if (target != kChanSendModeToAll && _userPtr == context.client)
			continue ;
		_userPtr->Send(_message);
		context.FDsPendingWrite.insert(_userPtr->GetFD());
	}
}

/**
 * @brief 사용자가 등록된 모든 채널에서 사용자를 삭제합니다.
 * 
 * @param[in,out] client 채널에서 삭제할 사용자의 포인터
*/
void IRCServer::RmClientFromChanJoined(IRCClient* client)
{
	std::string _nickname = client->GetNickname();
	
	// TODO IRCClient::ListChannels will return std::vector<std::string> in the future
	IRCClientJoinedChannels _joinedChannels = client->ListChannels();
	for (IRCClientJoinedChannels::iterator _it = _joinedChannels.begin(); _it != _joinedChannels.end(); _it++)
	{
		std::string _chan_name = _it->first;
		IRCChannel* _channel = GetChannel(_chan_name);
		if (_channel == NULL)
			continue ;
		
		// delete user from channel list
		_channel->DelChannelUser(_nickname);
		if(_channel->GetChannelUserSize() == 0)
			DelChannel(_channel->GetChannelInfo(kChannelName)); 
		// delete channel from user list
		client->DelInviteChannel(_chan_name);
	}
}

/**
 * @brief 사용자가 초대받은 모든 채널에서 사용자를 삭제합니다.
 * 
 * @param[in,out] client 채널에서 삭제할 사용자의 포인터
*/
void IRCServer::RmClientFromChanInvited(IRCClient* client)
{
	std::string _nickname = client->GetNickname();
	IRCClientChannels _invited_channels = client->ListInvitedChannels();

	for (IRCClientChannels::iterator _it = _invited_channels.begin(); _it != _invited_channels.end(); _it++)
	{
		std::string _chan_name = *_it;
		IRCChannel* channel = GetChannel(_chan_name);

		if (channel == NULL)
			continue ;
		// delete user from channel list
		channel->DelInvitedUser(_nickname);
		// delete channel from user list
		client->DelInviteChannel(_chan_name);
	}
}
