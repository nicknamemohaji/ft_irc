#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>

#include "TCPConnection.hpp"

struct IRCContext;

enum IRCClientActiveStatus
{
	REGISTER_PENDING,	// CAP, PASS
	REGISTER_ONGOING,	// USER, NICK
	REGISTERED			// registered
};

class IRCClient: public TCPConnection
{
	public:
		// constructor, destructor
		IRCClient(const int sockFd);
		~IRCClient(void);

		void OverwriteRecvBuffer(Buffer newBuffer);

		// getters
		enum IRCClientActiveStatus GetStatus(void) const;
		std::string GetNickname(void) const;
		std::string GetHostName(void) const;
		
		// setters
		void SetStatus(enum IRCClientActiveStatus newStatus);
		void SetNickName(const std::string& name);
		void SetHostName(const std::string& name);
	protected:

	private:

		enum IRCClientActiveStatus _activeStatus;

		std::string _nickname;
		std::string _host;
};

#endif