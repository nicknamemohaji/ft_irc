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

void IRCChannel::channelModeAdd(ChannelModeSet op){
	if(_channelMode & op)
		return;
	_channelMode |= op;
}
void IRCChannel::channelModeDel(ChannelModeSet op){
	if(_channelMode & op)
		_channelMode &= op;
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
	UserInChannel it = _userInChannel.find(client.GetNickname());
	if(it == _userInChannel.end() && it->second & op)
		return ;
	it->second = op;
}

void IRCChannel::setPasswd(const IRCClient &client, const std::string &pass){
	if(!isUserAuthorized(client,OPER))
		return;  // error msg ?
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
void IRCChannel::setChannelUser(const IRCClient &client, const unsigned int &num){
	channelModeAdd(LIMIT);
	_channelLimit = num;
}
void IRCChannel::addInviteUser(const IRCClient &client){
	if(!isInInvited(client))
		_userInvited.push_back(client.GetNickname());
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

void IRCChannel::addChannelUser(const IRCClient &client){
	if(channelModeCheck(INVITE) && !isInInvited(client))
		return;
	if(isInChannel(client))
		return;
	if(isInInvited(client))
	{
		std::vector<std::string>::iterator it = std::find(_userInvited.begin(),_userInvited.end(),client.GetNickname());
		if(it == _userInvited.end())
			return; // 초초대대대
	}
	_userInChannel.insert({client.GetNickname(),NOMAL});
}