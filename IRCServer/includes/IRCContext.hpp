#ifndef IRCSERVER_INCLUDES_IRCCONTEXT_HPP_
#define IRCSERVER_INCLUDES_IRCCONTEXT_HPP_

#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "IRCServer/includes/IRCTypes.hpp"

class IRCServer;
class IRCChannel;
class IRCClient;

struct IRCContext {
 public:
  // request
  bool createSource;
  enum IRCCommand command;
  std::deque<std::string> params;

  // context
  IRCServer* server;
  IRCChannel* channel;
  IRCClient* client;
  std::set<int>* pending_fds;

  // result
  int numericResult;
  std::string stringResult;

  // initialize struct...
  explicit IRCContext(std::set<int>* FDset);
  IRCContext(const IRCContext& other);

 private:
  IRCContext(void);
};

// for debug
std::ostream& operator<<(std::ostream& ostream, const IRCContext& context);

#endif  // IRCSERVER_INCLUDES_IRCCONTEXT_HPP_
