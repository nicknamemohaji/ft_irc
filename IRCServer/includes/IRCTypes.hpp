#ifndef IRCTYPES_HPP
# define IRCTYPES_HPP

#include <stdint.h>

#include <string>
#include <deque>
#include <vector>


enum IRCCommand
{
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

#endif