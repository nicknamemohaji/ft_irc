#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include "TCPConnection.hpp"

#include <string>
#include <map>

class IRCChannel;
struct IRCContext;

typedef std::map<std::string, IRCChannel*> IRCClientJoinedChannels;
typedef std::vector<std::string> IRCClientChannels;

enum IRCClientActiveStatus
{
	REGISTER_PENDING,	// pending PASS
	REGISTER_PASS,		// PASS is registered
	REGISTERED,			// registered
	PENDING_QUIT
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
		bool SetUserName(const std::string& name);

		//channel add, del, isinchannel
		void AddChannel(const std::string &channel_name, IRCChannel *channel);
		void DelChannel(const std::string &channel_name);
		bool IsInChannel(const std::string &channel_name);
		void AddInviteChannel(const std::string &channel_name);
		void DelInviteChannel(const std::string &channel_name);
		bool IsInviteChannel(const std::string &channel_name);
		bool IsInChannel(const std::string &channel_name) const;
		IRCClientJoinedChannels ListChannels(void) const;
		IRCClientChannels ListInvitedChannels(void) const;
		
	protected:

	private:

		enum IRCClientActiveStatus _activeStatus;

		IRCClientChannels _invited_channels_;
		// TODO change to std::vector<std::string>
		IRCClientJoinedChannels _channels;

		std::string _nickname;
		std::string _username;
};

#include "IRCChannel.hpp"
#endif