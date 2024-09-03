#include "TCPServer/includes/TCPConnection.hpp"

/**
 * @brief 전송 버퍼에 메시지를 삽입합니다
 *
 * @param[in] message 전송할 메시지
 *
 * @note Send 메소드는 버퍼에 메시지를 추가하기만 하므로
 * EVFILT_WRITE 이벤트가 발생하면
 * 멀티플렉서에서 SendBuffer 메소드를 호출해야 합니다
 */
template <typename T>
void TCPConnection::Send(const T &message) {
  if (!is_open_) throw TCPErrors::SocketClosed();
  buf_send_.insert(buf_send_.end(), message.begin(), message.end());
}
