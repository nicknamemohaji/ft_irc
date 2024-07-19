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

		// context actions
		void Context(IRCContext& context);

	protected:

	private:
		// setters
		void SetStatus(enum IRCClientActiveStatus newStatus);
		void SetNickName(const std::string& name);
		void SetHostName(const std::string& name);

		enum IRCClientActiveStatus _activeStatus;

		std::string _nickname;
		std::string _host;
};

#endif