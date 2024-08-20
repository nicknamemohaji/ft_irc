#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>
#include <map>

#include "TCPConnection.hpp"

class IRCChannel;

struct IRCContext;

enum IRCClientActiveStatus
{
	REGISTER_PENDING,	// pending PASS
	REGISTER_PASS,		// PASS is registered
	REGISTERED,			// registered
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
		std::string GetUserName(void) const;
		
		// setters
		void SetStatus(enum IRCClientActiveStatus newStatus);
		void SetNickName(const std::string& name);
		void SetUserName(const std::string& name);

		//channel add, del, isinchannel
		void AddChannel(const std::string &channel_name, IRCChannel *channel);
		void DelChannel(const std::string &channel_name);
		bool IsInChannel(const std::string &channel_name);
	protected:

	private:

		enum IRCClientActiveStatus _activeStatus;

		std::map<std::string, IRCChannel*> _channels;

		std::string _nickname;
		std::string _username;
};

#include "IRCChannel.hpp"
#endif