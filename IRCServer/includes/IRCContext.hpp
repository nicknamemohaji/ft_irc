#ifndef IRCCONTEXT_HPP
#define IRCCONTEXT_HPP

#include <string>
#include <vector>

class IRCServer;
class IRCChannel;
class IRCClient;

struct IRCContext
{
	// request
	std::string tag;
	std::string source;
	std::string command;
	std::vector<std::string> params;

	// context
	IRCServer* server;
	IRCChannel* channel;
	IRCClient* client;

	// numeric result
	int result;
	
	// initialize... 
	IRCContext(void)
	{
		tag.clear();
		source.clear();
		command.clear();
		params.clear();

		server = NULL;
		channel = NULL;
		client = NULL;
	}
};

#endif