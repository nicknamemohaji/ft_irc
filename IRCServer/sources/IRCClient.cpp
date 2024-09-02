#include "IRCServer/includes/IRCClient.hpp"

#include <iostream>
#include <map>

#include "IRCServer/includes/IRCTypes.hpp"

/**
 * @brief IRC 클라이언트를 추상화한 인스턴스를 생성합니다
 * 
 * @param[in] sockFD 연결된 클라이언트의 file descriptor
*/
IRCClient::IRCClient(const int sockFD)
    : TCPConnection(sockFD),
      active_status_(REGISTER_PENDING),
      nickname_(""),
      username_("") {}

/**
 * @brief IRC 클라이언트를 추상화한 인스턴스를 삭제합니다
 * 상위 클래스 TCPConnection의 소멸자에서 연결을 종료합니다
*/
IRCClient::~IRCClient(void) {}

/**
 * @brief 수신된 버퍼를 덮어씁니다
 * 
 * @param[in] newbuffer 덮어쓸 메시지
*/
void IRCClient::OverwriteRecvBuffer(Buffer newbuffer) {
  _recvBuf.clear();
  _recvBuf = newbuffer;
}

/**
 * @brief 클라이언트의 등록 상태를 변경합니다.
 * TCP 연결 수립 후, IRC 클라이언트의 등록 상태에 따라
 * REGISTER_PENDING < REGISTER_PASS < REGISTERED로 변화하고
 * QUIT 메시지 이후 삭제를 대기할 때는 PENDING_QUIT으로 설정됩니다
 * 
 * @param[in] status 변경할 상태
 * 
 * @note 상위 클래스인 TCPConnection의 소켓 상태와는 별개의 상태입니다
*/
void IRCClient::SetStatus(enum IRCClientActiveStatus status) {
  active_status_ = status;
}

/**
 * @brief USER 메시지 이후 클라이언트의 이름을 설정합니다
 * 
 * @param[in] name 변경할 이름
 * 
 * @return 처음 설정할 때면 true를 반환
 * 
 * @note IRC의 username은 name, host name, real name으로 구성되지만
 * 저희 구현에서는 name + IP 주소만 사용합니다
*/
bool IRCClient::SetUserName(const std::string &name) {
  if (username_.size() == 0) {
    username_ = name;
    return true;
  } else {
    return false;
  }
}

/**
 * @brief NICK 메시지 이후 닉네임을 변경합니다
 * 
 * @param[in] name 변경할 닉네임
*/
void IRCClient::SetNickName(const std::string &name) {
  nickname_ = name;
}

/**
 * @brief 클라이언트의 등록 상태를 확인합니다
 * 
 * @return 등록 상태(IRCClientActiveStatus)
*/
enum IRCClientActiveStatus IRCClient::GetStatus(void) const {
  return active_status_;
}

/**
 * @brief 클라이언트의 닉네임을 확인합니다
 * 
 * @return 닉네임이 설정되어 있지 않으면 `*`를, 설정되어 있으면 닉네임
*/
std::string IRCClient::GetNickname(void) const {
  if (nickname_.length() == 0)
    return "*";
  return nickname_;
}

/**
 * @brief 클라이언트의 이름을 반환합니다
 * 
 * @return 이름이 설정되어 있지 않으면 `~{nickname}`을, 설정되어 있으면 `~{username}`
*/
std::string IRCClient::GetUserName(void) const {
  if (username_.length() == 0)
    return "~" + nickname_;
  return "~" + username_;
}

/**
 * @brief 접속한 채널 목록에 채널을 추가합니다
 * 
 * @param[in] channel_name 채널 이름
*/
void IRCClient::AddChannel(const std::string &channel_name) {
  channels_.push_back(channel_name);
}

/**
 * @brief 접속한 채널 목록에서 채널을 삭제합니다
 * 
 * @param[in] channel_name 채널 이름
*/
void IRCClient::DelChannel(const std::string &channel_name) {
  IRCClientChannels::iterator _it = std::find(
    channels_.begin(), channels_.end(), channel_name);
  if (_it != channels_.end())
    channels_.erase(_it);
}

/**
 * @brief 접속한 채널 목록에 있는지 확인합니다
 * 
 * @param[in] channel_name 채널 이름
*/
bool IRCClient::IsInChannel(const std::string &channel_name) const {
  IRCClientChannels::const_iterator _it = std::find(
    channels_.begin(), channels_.end(), channel_name);
  return _it != channels_.end();
}

/**
 * @brief 초대받은 채널 목록에 채널을 추가합니다
 * 
 * @param[in] channel_name 채널 이름
*/
void IRCClient::AddInvitedChannel(const std::string &channel_name) {
  if (IsInInvitedChannel(channel_name)) return;
  invited_channels_.push_back(channel_name);
}

/**
 * @brief 초대받은 채널 목록에서 채널을 삭제합니다
 * 
 * @param[in] channel_name 채널 이름
*/
void IRCClient::DelInvitedChannel(const std::string &channel_name) {
  IRCClientChannels::iterator _it = std::find(
      invited_channels_.begin(), invited_channels_.end(), channel_name);
  if (_it != invited_channels_.end())
    invited_channels_.erase(_it);
}

/**
 * @brief 초대받은 채널 목록에 있는지 확인합니다
 * 
 * @param[in] channel_name 채널 이름
*/
bool IRCClient::IsInInvitedChannel(const std::string &channel_name) const {
  std::vector<std::string>::const_iterator it = std::find(
      invited_channels_.begin(), invited_channels_.end(), channel_name);
  return it != invited_channels_.end();
}

/**
 * @brief 접속한 채널 목록을 반환합니다
 *
 * @return 채널 목록
*/
IRCClientChannels IRCClient::ListChannels(void) const {
  return channels_;
}

/**
 * @brief 초대받은 채널 목록을 반환합니다
 *
 * @return 채널 목록
*/
IRCClientChannels IRCClient::ListInvitedChannels(void) const {
  return invited_channels_;
}
