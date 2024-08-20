#ifndef IRCCONTEXT_HPP
#define IRCCONTEXT_HPP

#include <string>
#include <deque>
#include <set>
#include <iostream>

class IRCServer;
class IRCChannel;
class IRCClient;

enum IRCCommand
{
	UNKNOWN = -1,
	// registration
	CAP,
	PASS,
	USER,
	NICK,
	// server informatin
	MOTD,
	PING,
	JOIN,
	KICK,
	PRIVMSG,
};

struct IRCContext
{
	public:

		// request
		std::string source;
		enum IRCCommand command;
		std::deque<std::string> params;
		std::string rawMessage;

		// context
		IRCServer* server;
		IRCChannel* channel;
		IRCClient* client;
		std::set<int>& FDsPendingWrite;

		// result
		int numericResult;
		std::string stringResult;
		
		// initialize struct... 
		IRCContext(std::set<int>& FDset);

		// converter
		static std::string ConvertCommandToStr(enum IRCCommand command);
		static enum IRCCommand ConvertStrToCommand(const std::string& command);

	private:
		IRCContext(void);
};

// for debug
std::ostream& operator<< (std::ostream& ostream, const IRCContext& context);

#endif