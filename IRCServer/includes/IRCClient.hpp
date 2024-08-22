#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include "TCPConnection.hpp"

#include <string>
#include <map>

class IRCChannel;
struct IRCContext;
typedef std::map<std::string, IRCChannel*> IRCClientChannels;

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
		void SetUserName(const std::string& name);

		//channel add, del, isinchannel
		void AddChannel(const std::string &channel_name, IRCChannel *channel);
		void DelChannel(const std::string &channel_name);
		bool IsInChannel(const std::string &channel_name) const;
		IRCClientChannels ListChannels(void) const;
		
	protected:

	private:

		enum IRCClientActiveStatus _activeStatus;
		// TODO change to std::vector<std::string>
		IRCClientChannels _channels;

		std::string _nickname;
		std::string _username;
};

#include "IRCChannel.hpp"
#endif