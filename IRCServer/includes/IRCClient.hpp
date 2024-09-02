#ifndef IRCSERVER_INCLUDES_IRCCLIENT_HPP_
#define IRCSERVER_INCLUDES_IRCCLIENT_HPP_

#include <map>
#include <string>

#include "TCPServer/includes/TCPConnection.hpp"
#include "IRCServer/includes/IRCTypes.hpp"

class IRCClient : public TCPConnection {
 public:
  // constructor, destructor
  explicit IRCClient(const int sockFd);
  ~IRCClient(void);

  void OverwriteRecvBuffer(Buffer newBuffer);

  // getters
  enum IRCClientActiveStatus GetStatus(void) const;
  std::string GetNickname(void) const;
  std::string GetUserName(void) const;

  // setters
  void SetStatus(enum IRCClientActiveStatus newStatus);
  void SetNickName(const std::string &name);
  bool SetUserName(const std::string &name);

  // channel add, del, isinchannel
  void AddChannel(const std::string &channel_name);
  void DelChannel(const std::string &channel_name);
  bool IsInChannel(const std::string &channel_name) const;
  void AddInvitedChannel(const std::string &channel_name);
  void DelInvitedChannel(const std::string &channel_name);
  bool IsInInvitedChannel(const std::string &channel_name) const;
  IRCClientChannels ListChannels(void) const;
  IRCClientChannels ListInvitedChannels(void) const;

 private:
  enum IRCClientActiveStatus active_status_;

  IRCClientChannels channels_;
  IRCClientChannels invited_channels_;

  std::string nickname_;
  std::string username_;
};

#endif  // IRCSERVER_INCLUDES_IRCCLIENT_HPP_
