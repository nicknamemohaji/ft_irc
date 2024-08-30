#ifndef IRCCONTEXT_HPP
#define IRCCONTEXT_HPP

#include "IRCTypes.hpp"

#include <string>
#include <deque>
#include <set>
#include <iostream>
#include <vector>

class IRCServer;
class IRCChannel;
class IRCClient;

// typedef std::vector<std::vector<std::string> > StringMatrix;
struct IRCContext
{
	public:
		// request
		bool createSource;
		enum IRCCommand command;
		std::deque<std::string> params;

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
	private:
		IRCContext(void);
};

// for debug
std::ostream& operator<< (std::ostream& ostream, const IRCContext& context);

#endif