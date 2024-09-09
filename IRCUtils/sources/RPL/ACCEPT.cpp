#include <sstream>
#include <string>

#include "IRCServer/includes/IRCClient.hpp"
#include "IRCServer/includes/IRCServer.hpp"
#include "IRCServer/includes/IRCTypes.hpp"
#include "IRCServer/includes/IRCContext.hpp"
#include "IRCUtils/includes/IRCResponseCreator.hpp"

void IRC_response_creator::RPL_ACCEPT(const IRCContext& context) {
  IRCContext _context(context);
  IRCClient* _client = const_cast<IRCClient*>(_context.client);
  const std::string _client_nickname = _client->GetNickname();
  const std::string _server_name = context.server->GetServerName();
  const std::string _start_date = context.server->GetServerStartDate();

  std::stringstream strstream;
  _context.createSource = false;

  // send RPL_WELCOME
  strstream.str("");
  _context.stringResult.clear();
  strstream << _client_nickname << " :Welcome to the " << _server_name
            << " Network, " << _client_nickname;
  _context.numericResult = 1;
  _context.stringResult = strstream.str();
  _context.client->Send(IRC_response_creator::MakeResponse(_context));

  // send RPL_YOURHOST
  strstream.str("");
  _context.stringResult.clear();
  strstream << _client_nickname << " :Your host is " << _server_name
            << ", running version " << VERSION;
  _context.numericResult = 2;
  _context.stringResult = strstream.str();
  _context.client->Send(IRC_response_creator::MakeResponse(_context));

  // send RPL_CREATED
  strstream.str("");
  _context.stringResult.clear();
  strstream << _client_nickname << " :This server was created " << _start_date;
  _context.numericResult = 3;
  _context.stringResult = strstream.str();
  _context.client->Send(IRC_response_creator::MakeResponse(_context));

  // send RPL_MYINFO
  strstream.str("");
  _context.stringResult.clear();
  strstream << _client_nickname << " " << _server_name << " " << VERSION
            << "o itlk";
  _context.stringResult = strstream.str();
  _context.numericResult = 4;
  _context.client->Send(IRC_response_creator::MakeResponse(_context));

  // send RPL_ISUPPORT
  strstream.str("");
  _context.stringResult.clear();
  strstream << _client_nickname
            << " CHANTYPES=# CASEMAPPING=ascii CHANMODES=,k,l,it "
            << "PREFIX=(o)@ NICKLEN=30 :are supported by this server";
  _context.stringResult = strstream.str();
  _context.numericResult = 5;
  _context.client->Send(IRC_response_creator::MakeResponse(_context));
}
