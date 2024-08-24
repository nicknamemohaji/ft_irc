#include <iostream>
#include <map>

#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCChannel.hpp"
#include "IRCErrors.hpp"

IRCClient::IRCClient(const int sockFD):
	TCPConnection(sockFD),
	_activeStatus(REGISTER_PENDING),
	_nickname(""),
	_username("")
{
	# ifdef DEBUG
	std::cout << "[INFO] IRCClient: Constructor: instnace created" << std::endl;
	# endif
}

IRCClient::~IRCClient(void)
{
	# ifdef DEBUG
	std::cout << "[INFO] TCPConnection: Destructor: instnace destroyed" << std::endl;
	# endif
}

/*************/

void IRCClient::OverwriteRecvBuffer(Buffer newBuffer)
{
	_recvBuf.clear();
	_recvBuf = newBuffer;
}

/*************/

void IRCClient::SetStatus(enum IRCClientActiveStatus newStatus)
{
	_activeStatus = newStatus;
}

void IRCClient::SetUserName(const std::string& name)
{
	if (_username.size() == 0)
		_username = name;
	else
		throw IRCError::AlreadyRegistered();
}

void IRCClient::SetNickName(const std::string& name)
{
	_nickname = name;
}

/*************/

enum IRCClientActiveStatus IRCClient::GetStatus(void) const
{
	return _activeStatus;
}

std::string IRCClient::GetNickname(void) const
{
	if (_nickname.length() == 0)
		return "*";
	return _nickname;
}

std::string IRCClient::GetUserName(void) const
{
	if (_username.length() == 0)
		return "~" + _nickname;
	return "~" + _username;
}

void IRCClient::AddChannel(const std::string &channel_name, IRCChannel *channel){
	_channels[channel_name] = channel;
}
void IRCClient::DelChannel(const std::string &channel_name){
	IRCClientJoinedChannels::iterator it = _channels.find(channel_name);
	if(it == _channels.end())
		return;
	_channels.erase(it);
}
bool IRCClient::IsInChannel(const std::string &channel_name) const {
	return _channels.find(channel_name) != _channels.end();
}

void IRCClient::AddInviteChannel(const std::string &channel_name){
	_invited_channels_.push_back(channel_name);
}
void IRCClient::DelInviteChannel(const std::string &channel_name){
	std::vector<std::string>::iterator it;
	for(it = _invited_channels_.begin(); it != _invited_channels_.end(); it++){
		if(*it == channel_name){
			_invited_channels_.erase(it);
			return;
		}
	}
}

bool IRCClient::IsInviteChannel(const std::string &channel_name){
	std::vector<std::string>::iterator it;
	for(it = _invited_channels_.begin(); it != _invited_channels_.end(); it++){
		if(*it == channel_name){
			return true;
		}
	}
	return false;
}

IRCClientJoinedChannels IRCClient::ListChannels(void) const
{
	return _channels;
}

IRCClientChannels IRCClient::ListInvitedChannels(void) const
{
	return _invited_channels_;
}
