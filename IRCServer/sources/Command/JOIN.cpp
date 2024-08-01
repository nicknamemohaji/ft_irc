#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

# ifndef VERSION
# define VERSION "42.42"
# endif

// void IRCServer::ActionJOIN(IRCContext& context)
// {
// 	std::stringstream result;

// 	///do join
// }