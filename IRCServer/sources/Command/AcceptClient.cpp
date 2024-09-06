#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>

#include "IRCServer/includes/IRCChannel.hpp"
#include "IRCServer/includes/IRCClient.hpp"
#include "IRCServer/includes/IRCContext.hpp"
#include "IRCServer/includes/IRCErrors.hpp"
#include "IRCServer/includes/IRCServer.hpp"
#include "IRCServer/includes/IRCTypes.hpp"
#include "IRCUtils/includes/IRCResponseCreator.hpp"

void IRCServer::ActionCAP(IRCContext& context) {
  // DO NOTHING
  (void) context;
}

void IRCServer::ActionPASS(IRCContext& context) {
  IRCClient* _client = const_cast<IRCClient*>(context.client);
  if (_client->GetStatus() == REGISTER_PENDING) {
    if (!(context.params.size() == 1 && context.params[0] == _serverPass)) {
      return IRC_response_creator::ERR_PASSWDMISMATCH(_client, server_name_,
                                                      context.pending_fds);
    }
    _client->SetStatus(REGISTER_PASS);
    return;
  } else {
    return IRC_response_creator::ERR_ALREADYREGISTERED(_client, server_name_,
                                                       context.pending_fds);
  }
}

void IRCServer::ActionNICK(IRCContext& context) {
  IRCContext _context(context);
  IRCClient* _client = const_cast<IRCClient*>(context.client);

  // (1) client has not validated password
  if (_client->GetStatus() == REGISTER_PENDING) {
    return IRC_response_creator::ERR_PASSWDMISMATCH(_client, server_name_,
                                                    _context.pending_fds);
  }

  // validate parameter
  if (_context.params.size() == 0) {
    return IRC_response_creator::ERR_NONICKNAMEGIVEN(_client, server_name_,
                                                     _context.pending_fds);
  } else if (_context.params.size() > 1) {
    return IRC_response_creator::ERR_ERRONEUSNICKNAME(_client, server_name_,
                                                      _context.pending_fds);
  }
  // validate nickname
  std::string _new_name = _context.params[0];
  if (GetClient(_new_name) != NULL) {
    return IRC_response_creator::ERR_NICKNAMEINUSE(_client, server_name_,
                                                   _context.pending_fds);
  }
  if (!(2 <= _new_name.size() && _new_name.size() <= 30)) {
    return IRC_response_creator::ERR_ERRONEUSNICKNAME(_client, server_name_,
                                                      _context.pending_fds);
  }
  for (unsigned int i = 0; i < _new_name.size(); i++) {
    if (!std::isalnum(static_cast<unsigned char>(_new_name[i]))) {
      return IRC_response_creator::ERR_ERRONEUSNICKNAME(_client, server_name_,
                                                        _context.pending_fds);
    }
  }

  // good to go!
  if (_client->GetStatus() == REGISTER_PASS) {
    // (2) initial register
    // set status
    _client->SetStatus(REGISTERED);
    _client->SetNickName(_new_name);
    // send accept RPLs
    IRC_response_creator::RPL_ACCEPT(context);
    ActionMOTD(context);
  } else {
    // (3) change nickname
    std::string _prev_name = _client->GetNickname();
    // send acknowledgement to user
    _context.createSource = true;
    _context.numericResult = -1;
    _context.stringResult = _new_name;
    _client->Send(IRC_response_creator::MakeResponse(_context));
    // make change in joined servers
    IRCClientChannels _joined_channels = _client->ListChannels();
    for (IRCClientChannels::iterator _it = _joined_channels.begin();
         _it != _joined_channels.end(); _it++) {
      IRCChannel* _joined_channel = GetChannel(*_it);
      _context.channel = _joined_channel;
      SendMessageToChannel(kChanSendModeToExceptMe, _context);
      _joined_channel->ChangeChannelUser(_prev_name, _new_name);
    }
    // make change in invited servers
    IRCClientChannels _invited_channels = _client->ListInvitedChannels();
    for (IRCClientChannels::iterator _it = _invited_channels.begin();
         _it != _invited_channels.end(); _it++) {
      IRCChannel* _invited_channel = GetChannel(*_it);
      // change name from channel
      _invited_channel->DelInvitedUser(_prev_name);
      _invited_channel->AddInvitedUser(_new_name);
    }

    // remove previous nickname
    _clients.erase(_clients.find(_client->GetNickname()));
  }

  // Add new name
  _clients[_new_name] = _client;
  _client->SetNickName(_new_name);
  _context.pending_fds->insert(_client->GetFD());
}

void IRCServer::ActionUSER(IRCContext& context) {
  IRCContext _context(context);
  IRCClient* _client = const_cast<IRCClient*>(context.client);

  if (_client->GetStatus() == REGISTER_PENDING) {
    return IRC_response_creator::ERR_PASSWDMISMATCH(_client, server_name_,
                                                    _context.pending_fds);
  }
  if (_context.params.size() != 4) {
    return IRC_response_creator::ERR_NEEDMOREPARAMS(
        _client, server_name_, _context.pending_fds, _context.command);
  }

  if (!_client->SetUserName(_context.params[0])) {
    return IRC_response_creator::ERR_ALREADYREGISTERED(_client, server_name_,
                                                       _context.pending_fds);
  }

  _context.pending_fds->insert(_client->GetFD());
}

/**************/

void IRCServer::ActionMOTD(IRCContext& context) {
  if (context.command == MOTD) {
    for (std::deque<std::string>::iterator it = context.params.begin();
         it != context.params.end(); it++) {
      if (*it != server_name_) {
        return IRC_response_creator::ERR_NOSUCHSERVER(
            context.client, server_name_, context.pending_fds, *it);
      }
    }
  }
  std::string clientNickname = context.client->GetNickname();

  context.stringResult =
      clientNickname + " : -- Welcome to " + server_name_ + "--";
  context.numericResult = 375;
  context.client->Send(IRC_response_creator::MakeResponse(context));

  context.stringResult = clientNickname + " :Mesasge of the day:";
  context.numericResult = 372;
  context.client->Send(IRC_response_creator::MakeResponse(context));

  context.stringResult = clientNickname + " :WeLOve42Seoul";
  context.numericResult = 372;
  context.client->Send(IRC_response_creator::MakeResponse(context));

  context.stringResult = clientNickname + " :end of MOTD";
  context.numericResult = 376;
  context.client->Send(IRC_response_creator::MakeResponse(context));

  context.pending_fds->insert(context.client->GetFD());
}
