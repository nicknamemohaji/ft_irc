#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>

#include "TCPConnection.hpp"

class IRCClient: public TCPConnection
{
	public:
		IRCClient(const int sockFd);
		~IRCClient(void);

	protected:

	private:
		bool _isActive;

		std::string _nickname;
		std::string _hostName;
		std::string _connectedServerName;
};

#endif