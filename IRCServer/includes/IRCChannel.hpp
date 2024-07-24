#ifndef IRCCHANNEL_HPP
#define IRCCHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include "IRCClient.hpp"

#define MAX_CHANNEL_USER 100

typedef enum ChannelPermition{
	OPER = 1 << 0, //오퍼레이터 권한
	NOMAL = 1 << 1 //일반 권한
};

typedef enum ChannelModeSet{
	LIMIT = 1 << 0,  // 채널의 사용자 리밋 제한설정 -> 설정시 리미트 확인
	INVITE = 1 << 1,  // 초대전용채널 설정 -> 설정시 초대 되었는지 확인
	TOPIC = 1 << 2,  // topic 명령어 운영자에게만 설정 -> 설정시 운영자만 변경 아니면 모두가능
	PASS = 1 << 3,  // 채널 비밀번호 설정 -> 설정시 비밀번호 확인
	//o   // 채널 운영자 권한부여
};

typedef std::map<std::string, ChannelPermition>::iterator UserInChannel; 
class IRCChannel
{
	public:
	IRCChannel(void);
	IRCChannel(const std::string &_name, const IRCClient &client);
	~IRCChannel(void);
	void channelModeAdd(const IRCClient &client ,ChannelModeSet op);  //없다면 추가 있다면 제거.
	void channelModeDel(const IRCClient &client ,ChannelModeSet op);  //없다면 추가 있다면 제거.
	bool channelModeCheck(ChannelModeSet op) const ;  //해당모드인지 확인
	bool isUserAuthorized(const IRCClient &client ,ChannelPermition op); // clinet가 채널의 op권한 있는지 확인
	void setUserAuthorized(const IRCClient &client ,ChannelPermition op); // client 가 채널의 op설정
	void setPasswd(const IRCClient &client, const std::string &pass);  // client 가 channel의 passwd설정
	void setTopic(const IRCClient &client, const std::string &topic); // client 가 channel의 topic 설정
	void setChannelUserLimit(const IRCClient &client, const unsigned int &num);  // client 가 channel의 최대 인원 설정
	void addInviteUser(const IRCClient &client, const IRCClient &target);  // clinet 가 target 유저 초대리스트에 추가
	bool isInChannel(const IRCClient &client) const; // client가 채널에 있는지 확인
	bool isInInvited(const IRCClient &client) const ; // client가 초대 목록에 있는지 확인
	bool matchPasswd(const std::string &passwd) const; // channel의 패스워드와 일치하는지 확인
	void addChannelUser(const IRCClient &client, const IRCClient &target); // 채널에 client를 추가
	void manageChannelPermit(const IRCClient &client, const IRCClient &target, ChannelPermition op); //채널의 OP 권한을 해당유저에게 줌
	private:
	std::vector<std::string> _userInvited; // 초대된 유저리스트
	std::map<std::string, ChannelPermition> _userInChannel; // 채널에 있는 유저 리스트
	unsigned int _channelMode; // 채널에 설정되어있는 모드. 비트마스킹
	unsigned int _channelLimit; // 채널 맥시멈 유저수
	std::string _channelName; // 채널이름 
	std::string _channelTopic; // 채널토픽
	std::string _channelPasswd; // 채널 패스워드


};

#endif