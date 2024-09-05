#include "IRCServer/includes/IRCContext.hpp"

#include <deque>
#include <iostream>
#include <string>

IRCContext::IRCContext(std::set<int>* fdset)
    : createSource(false),
      command(UNKNOWN),
      server(NULL),
      channel(NULL),
      client(NULL),
      pending_fds(fdset),
      numericResult(-1),
      stringResult("") {
  params.clear();
}

IRCContext::IRCContext(const IRCContext& other)
    : createSource(other.createSource),
      command(other.command),
      params(other.params),
      server(other.server),
      channel(other.channel),
      client(other.client),
      pending_fds(other.pending_fds),
      numericResult(other.numericResult),
      stringResult(other.stringResult) {}

std::ostream& operator<<(std::ostream& ostream, const IRCContext& context) {
  ostream << "\n--- IRCContext Dump ---\n";
  ostream << "command: [" << context.command << "]" << std::endl;
  ostream << "params: " << std::endl;
  for (std::deque<std::string>::const_iterator it = context.params.begin();
       it != context.params.end(); it++)
    ostream << "-- [" << *it << "]" << std::endl;
  ostream << "numericResult: " << context.numericResult << std::endl;
  ostream << "stringResult: " << context.stringResult << std::endl;

  return ostream;
}
