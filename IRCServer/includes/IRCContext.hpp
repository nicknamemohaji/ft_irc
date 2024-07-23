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
	UNKNOWN,
	// registration
	CAP,
	PASS,
	USER,
	NICK,
	// server informatin
	MOTD,
	PING,
	JOIN,
};

struct IRCContext
{
	public:

		// request
		std::string source;
		enum IRCCommand command;
		std::deque<std::string> params;
		std::string rawMessage;
		/*
		notes on IRCContext.source:

		클라이언트 요청에는 source 정보가 없지만, 서버가 클라이언트로 보내는 정보에는 source가 포함될 수 있습니다.
		(ex. PRIVMSG: 발신자 정보를 source로 포함)

		IRCContext.source 정보는 IRCServer::Context에서 등록됩니다
		*/

		// context
		IRCServer* server;
		IRCChannel* channel;
		IRCClient* client;

		// result
		int numericResult;
		std::string stringResult;
		std::set<int>& FDsPendingWrite;
		
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