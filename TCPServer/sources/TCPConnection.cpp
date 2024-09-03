#include "TCPServer/includes/TCPConnection.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "TCPServer/includes/TCPErrors.hpp"

// ===== constructor, destructor =====
/**
 * @brief 클라이언트 소켓을 추상화합니다
 *
 * @param[in] sock_fd 연결된 클라이언트의 file descriptor
 *
 * @throw TCPErrors::SystemCallError
 * 클라이언트의 정보를 받아오고 IO 모드를 변경하는 과정에서 오류가 발생하면
 * exception이 발생합니다
 */
TCPConnection::TCPConnection(const int sock_fd)
    : socket_(sock_fd), is_open_(true) {
  // ***** fetch client information *****
  // use getpeername(2) to fetch peer's information
  struct sockaddr_storage clientInfo;
  socklen_t clientInfoLen = sizeof(clientInfo);
  int err = getpeername(socket_,                        // sockfd
                        (struct sockaddr*)&clientInfo,  // addr
                        &clientInfoLen);                // len
  if (err != 0) throw TCPErrors::SystemCallError("getpeername(2)");
  // use inet_ntop(3) to convert peer's ip address to c-style string
  char buf[256];
  std::memset(buf, 0, sizeof(buf));
  if (inet_ntop(AF_INET, &(((struct sockaddr_in*)&clientInfo)->sin_addr), buf,
                sizeof(buf)) == NULL)
    throw TCPErrors::SystemCallError("inet_ntop(3)");
  client_ip_ = buf;

  // ***** set socket fd *****
  // make fd O_NONBLOCK
  err = fcntl(socket_,      // fd
              F_SETFL,      // command: F_SETFL = set file status
              O_NONBLOCK);  // argument: O_NONBLOCK = non-blocking
  if (err) throw TCPErrors::SystemCallError("fcntl(2)");

  // initialize buffer
  buf_send_.clear();
  buf_recv_.clear();
}

/**
 * @brief 소켓 연결을 닫고 인스턴스를 소멸합니다
 */
TCPConnection::~TCPConnection(void) { Close(); }

/**
 * @brief 소켓 연결을 닫고 상태(is_open_)를 변경합니다
 */
void TCPConnection::Close(void) {
  if (!is_open_) return;

  // send FIN packet
  int res = shutdown(socket_,     // sockfd
                     SHUT_RDWR);  // how = close all connection

  if (res) TCPErrors::SystemCallError("shutdown(2)");

  // close socket
  res = close(socket_);
  if (res) TCPErrors::SystemCallError("close(2)");

  // mark status
  is_open_ = false;
}

// ===== socket IO actions =====
// 1. Recv related methods
/**
 * @brief 메시지를 수신하고 버퍼에 추가합니다
 *
 * @note non-blocking 소켓이므로 반드시 멀티플렉서를 통해 이벤트가 있는 것을
 * 확인하고 Recv 메소드를 호출해야 합니다.
 * (EAGAIN 발생 - MSG_NOSIGNAL 옵션이 맥에는 없음)
 */
void TCPConnection::Recv(void) {
  if (!is_open_) throw TCPErrors::SocketClosed();

  uint8_t buf[TCPCONN_BUF_SIZE];
  std::memset(buf, 0, sizeof(buf));

  int err = recv(socket_,      // sockfd
                 buf,          // buf
                 sizeof(buf),  // len
                 0);           // flags: none
  if (err == -1) {
    // system call error
    // can't check errno due to subject restriction :(
    throw TCPErrors::SystemCallError("recv(2)");
  } else if (err == 0) {
    // peer closed connection
    Close();
    return;
  }

  buf_recv_.insert(buf_recv_.end(), buf, buf + err);
}

/**
 * @brief 수신 버퍼의 길이를 반환합니다
 *
 * @return 수신 버퍼의 길이(바이트 단위)
 */
int TCPConnection::GetRecvBufferSize(void) const { return buf_recv_.size(); }

/**
 * @brief 수신 버퍼를 반환합니다
 *
 * @return 수신 버퍼
 */
Buffer TCPConnection::ReadRecvBuffer(void) const { return buf_recv_; }

/**
 * @brief 수신 버퍼를 초기화합니다
 */
void TCPConnection::ClearRecvBuffer(void) { buf_recv_.clear(); }

/**
 * @brief 수신할 메시지가 더 있는지 확인합니다
 * (short count 대응)
 *
 * @return 메시지를 끝까지 읽었는지 여부
 */
bool TCPConnection::CheckRecvEnd(void) const {
  if (!is_open_) throw TCPErrors::SocketClosed();

  uint8_t _byte;
  int _res = recv(socket_,        // sockfd
                  &_byte,         // buf
                  sizeof(_byte),  // len
                  MSG_PEEK);      // flags: MSG_PEEK = just check buffer
  return _res != sizeof(_byte);
}

// 2. Send related methods

/**
 * @brief 실제로 클라이언트에게 전송 버퍼에 있는 메시지를 전달합니다
 *
 * @note 서브젝트 제한상 errno를 확인할 수 없으므로,
 * 멀티플렉서에서 EVFILT_WRITE 이벤트를 확인한 후에만 SendBuffer 메소드를
 * 호출해야 합니다
 */
void TCPConnection::SendBuffer(void) {
  if (!is_open_) throw TCPErrors::SocketClosed();

  int _send_size = buf_send_.size();
  if (_send_size > TCPCONN_BUF_SIZE) _send_size = TCPCONN_BUF_SIZE;
  int err = send(socket_,           // sockfd
                 buf_send_.data(),  // msg
                 _send_size,        // len
                 0);                // flags: 0
  if (err == -1) {
    // error occured
    throw TCPErrors::SystemCallError("send(2)");
  } else {
    // sent succesfully
    buf_send_.erase(buf_send_.begin(), buf_send_.begin() + err);
  }
}

/**
 * @brief 전송 버퍼의 크기를 확인합니다. 이 값을 이용해 EVFILT_WRITE 이벤트를 더
 * 감지할 지 결정할 수 있습니다
 *
 * @return 전송 버퍼의 크기
 */
int TCPConnection::GetSendBufferSize(void) const { return buf_send_.size(); }

// ===== getters =====
/**
 * @brief 클라이언트의 IP 주소를 반환합니다
 *
 * @return 클라이언트의 IP 주소
 */
std::string TCPConnection::GetIP(void) const {
  if (!is_open_) throw TCPErrors::SocketClosed();
  return client_ip_;
}

/**
 * @brief 소켓의 file descriptor를 반환합니다
 *
 * @return 소켓의 FD 번호
 */
int TCPConnection::GetFD(void) const { return socket_; }

// ===== utility function =====

/**
 * @brief Buffer 자료형을 std::string으로 변환합니다
 *
 * @param[in] buf 변환할 버퍼
 *
 * @return 변환된 문자열
 */
std::string TCPConnection::BufferToString(const Buffer& buf) {
  std::string ret;
  ret.assign(buf.begin(), buf.end());
  return ret;
}
