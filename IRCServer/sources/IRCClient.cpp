#include <iostream>

#include "IRCClient.hpp"
#include "IRCContext.hpp"

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

void IRCClient::Context(IRCContext& context)
{
	switch (context.command)
	{
		case PASS:
			SetStatus(REGISTER_ONGOING);
			return ;
		case USER:
			SetHostName(context.params[0]);
			if (_nickname.size() != 0)
				SetStatus(REGISTERED);
			return ;
		case NICK:
			SetNickName(context.params[0]);
			if (_host.size() != 0)
				SetStatus(REGISTERED);
			return;
		default:
			throw std::logic_error("NotImplementedError");
	}
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