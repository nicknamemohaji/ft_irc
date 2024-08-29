#ifndef IRCSERVER_INCLUDES_IRCTYPES_HPP_
#define IRCSERVER_INCLUDES_IRCTYPES_HPP_

#include <stdint.h>

#include <string>
#include <deque>
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
  JOIN,
  NAMES,
  MODE,
  PART,
  TOPIC,
  KICK,
  PRIVMSG,
  INVITE,
};

typedef std::vector<uint8_t> Buffer;
typedef std::deque<std::string> IRCParams;

#endif  // IRCSERVER_INCLUDES_IRCTYPES_HPP_
