#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>
#include <map>

#include "TCPConnection.hpp"

class IRCChannel;

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

		//channel add, del, isinchannel
		void AddChannel(const std::string &channel_name, IRCChannel *channel);
		void DelChannel(const std::string &channel_name);
		bool IsInChannel(const std::string &channel_name);
		void AddInviteChannel(const std::string &channel_name);
		void DelInviteChannel(const std::string &channel_name);
		bool IsInviteChannel(const std::string &channel_name);
	protected:

	private:

		enum IRCClientActiveStatus _activeStatus;

		std::map<std::string, IRCChannel*> _channels;
		std::vector<std::string> _invited_channels_;

		std::string _nickname;
		std::string _host;
};

#include "IRCChannel.hpp"
#endif