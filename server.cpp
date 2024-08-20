#include <iostream>
#include <string>

#include <vector>

#include "TCPMultiplexer.hpp"
#include "IRCServer.hpp"

int main(int argc, const char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv {PORT} {PASSWORD}" << std::endl;
		return 1;
	}

	std::cout << "===== Simple IRC Server =====" << std::endl;
	IRCServer* server1 = new IRCServer(argv[1], "ft_irc", argv[2]);

	std::vector<TCPServer*> servers;
	servers.push_back(server1);
	TCPMultiplexer multiplexer(servers);

	while (true)
	{
		# ifdef DEBUG
		std::cout << "[*] waiting for event" << std::endl;
		# endif
		multiplexer.WaitEvent();
	}

	for (std::vector<TCPServer*>::iterator it = servers.begin(); it != servers.end(); it++)
		delete *it;
}
