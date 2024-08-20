#include <iostream>
#include <map>

#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCChannel.hpp"

IRCClient::IRCClient(const int sockFD):
	TCPConnection(sockFD),
	_activeStatus(REGISTER_PENDING),
	_nickname(""),
	_host("")
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

void IRCClient::SetHostName(const std::string& name)
{
	_host = name;
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

std::string IRCClient::GetHostName(void) const
{
	if (_host.length() == 0)
		return "*";
	return _host;
}

void IRCClient::AddChannel(const std::string &channel_name, IRCChannel *channel){
	_channels[channel_name] = channel;
}
void IRCClient::DelChannel(const std::string &channel_name){
	std::map<std::string, IRCChannel*>::iterator it = _channels.find(channel_name);
	if(it == _channels.end())
		return;
	_channels.erase(it);
}
bool IRCClient::IsInChannel(const std::string &channel_name){
	return _channels.find(channel_name) != _channels.end();
}