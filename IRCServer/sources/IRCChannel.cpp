#include "IRCChannel.hpp"

IRCChannel::IRCChannel(void){
	_channelName = "";
	_channelLimit = MAX_CHANNEL_USER;
	_channelMode = 0;
	_channelTopic = "";
	_channelPasswd = "";
	_userInvited.clear();
	_userInChannel.clear();
}
IRCChannel::IRCChannel(const std::string &_name, const IRCClient &client) : _channelName(_name){
	_channelLimit = MAX_CHANNEL_USER;
	_channelMode = 0;
	_channelTopic = "";
	_channelPasswd = "";
	_userInvited.clear();
	_userInChannel.clear();
	_userInChannel.insert({client.GetNickname(), OPER});
}

void IRCChannel::channelModeAdd(const IRCClient &client ,ChannelModeSet op){
	if(!isUserAuthorized(client,OPER) || _channelMode & op)
		return;
	_channelMode |= op;
}
void IRCChannel::channelModeDel(const IRCClient &client ,ChannelModeSet op){
	if(!isUserAuthorized(client, OPER))
		return;
	if(_channelMode & op)
	{
		_channelMode &= op;
		if(op == LIMIT)
			_channelLimit = MAX_CHANNEL_USER;
		else if(op == PASS)
			_channelPasswd = "";
	}
}
bool IRCChannel::channelModeCheck(ChannelModeSet op) const {
	if(_channelMode & op)
		return true;
	return false;
}

bool IRCChannel::isUserAuthorized(const IRCClient &client ,ChannelPermition op) {
	UserInChannel it = _userInChannel.find(client.GetNickname());
	if(it == _userInChannel.end())
		return false;
	if(it->second & op)
		return true;
	return false;
}

void IRCChannel::setUserAuthorized(const IRCClient &client ,ChannelPermition op){
	if(!isUserAuthorized(client,OPER))
		return; // 권한ㅇ벗음.
	UserInChannel it = _userInChannel.find(client.GetNickname());
	if(it == _userInChannel.end() && it->second & op)
		return ; // 유저가 채널에 없음
	it->second = op;
}

void IRCChannel::setPasswd(const IRCClient &client, const std::string &pass){
	if(!isUserAuthorized(client,OPER))
		return;  // error msg ? 권한없음
	_channelPasswd = pass;
}
void IRCChannel::setTopic(const IRCClient &client, const std::string &topic){
	if(channelModeCheck(TOPIC) && isUserAuthorized(client,OPER))
	{
		_channelTopic = topic;
		return;
	}
	else if(!channelModeCheck(TOPIC))
	{
		_channelTopic = topic;
		return;
	}
}
void IRCChannel::setChannelUserLimit(const IRCClient &client, const unsigned int &num){
	channelModeAdd(client, LIMIT);
	_channelLimit = num;
}
void IRCChannel::addInviteUser(const IRCClient &client, const IRCClient &target){
	if(isInChannel(client) && !isInInvited(target))
		_userInvited.push_back(target.GetNickname());
}
bool IRCChannel::isInChannel(const IRCClient &client) const {
	if(_userInChannel.find(client.GetNickname()) == _userInChannel.end())
		return false;
	return true;
}
bool IRCChannel::isInInvited(const IRCClient &client) const {
	if(std::find(_userInvited.begin(),_userInvited.end(),client.GetNickname()) == _userInvited.end())
		return false;
	return true;
}

bool IRCChannel::matchPasswd(const std::string &passwd) const {
	if(channelModeCheck(PASS) && passwd != _channelPasswd)
		return false;
	return true;
}

void IRCChannel::addChannelUser(const IRCClient &client, const IRCClient &target){
	if(!isInChannel(client))
		return; //초대자가 채널에 없음.
	if(channelModeCheck(LIMIT) && _userInChannel.size() >= _channelLimit)
		return; // 채널인원 초과 error??
	if(channelModeCheck(INVITE) && !isInInvited(target))
		return; // 초대모드 이지만 초대목록에 없음. error 처리?
	if(isInChannel(target))
		return; // 채널에 이미 있음.
	if(isInInvited(target))
	{
		std::vector<std::string>::iterator it = std::find(_userInvited.begin(),_userInvited.end(),target.GetNickname());
		_userInvited.erase(it);
		//채널 접속후 초대리스트에서 삭제.
	}
	_userInChannel.insert({target.GetNickname(),NOMAL});
	//초대리스트에 추가
}

void IRCChannel::manageChannelPermit(const IRCClient &client, const IRCClient &target, ChannelPermition op){
	if(!isUserAuthorized(client,OPER))
		return; // 권한이 없음
	setUserAuthorized(target, op);
}