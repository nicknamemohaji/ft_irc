#ifndef IRCSERVER_INCLUDES_IRCTYPES_HPP_
#define IRCSERVER_INCLUDES_IRCTYPES_HPP_

#include <stdint.h>

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

enum IRCCommand {
  UNKNOWN = -1,
  // registration
  CAP,
  PASS,
  USER,
  NICK,
  QUIT,
  // server informatin
  MOTD,
  PING,
  // channel actions
  JOIN,
  NAMES,
  MODE,
  PART,
  TOPIC,
  KICK,
  INVITE,
  // send message
  PRIVMSG,
};

enum IRCClientActiveStatus {
  REGISTER_PENDING,  // pending PASS
  REGISTER_PASS,     // PASS is registered
  REGISTERED,        // registered
  PENDING_QUIT
};

typedef std::vector<uint8_t> Buffer;
typedef std::deque<std::string> IRCParams;
typedef std::set<int> FDSet;
typedef std::vector<std::string> IRCClientChannels;

#endif  // IRCSERVER_INCLUDES_IRCTYPES_HPP_
