#include <iostream>
#include <string>

#include <vector>

#include "TCPMultiplexer.hpp"
#include "IRCServer.hpp"

int main(void)
{

	std::cout << "===== Simple IRC Server =====" << std::endl;
	IRCServer* server1 = new IRCServer("6667", "ft_irc", "verystrongpassword");

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
