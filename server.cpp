#include <iostream>
#include <string>
#include <vector>

#include "IRCServer/includes/IRCServer.hpp"
#include "TCPServer/includes/TCPMultiplexer.hpp"

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./ircserv {PORT} {PASSWORD}" << std::endl;
    return 1;
  }

  std::cout << "===== Simple IRC Server =====" << std::endl;
  IRCServer* server1 = new IRCServer(argv[1], "ft_irc", argv[2]);

  std::vector<TCPServer*> servers;
  servers.push_back(server1);
  TCPMultiplexer multiplexer(servers);

  while (true) {
    multiplexer.WaitEvent();
  }

  for (std::vector<TCPServer*>::iterator it = servers.begin();
       it != servers.end(); it++)
    delete *it;
}
