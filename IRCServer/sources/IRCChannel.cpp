#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>
#include <sstream>
#include <deque>
#include <queue>
#include <iostream>

#include "IRCChannel.hpp"


IRCChannel::IRCChannel(const std::string& nickname, const std::string& channel_name) {
    SetChannelInfo(kChannelName,channel_name);
    SetChannelInfo(kChannelDate,itostr(std::time(NULL)));
    SetChannelInfo(kTopicEditTime,itostr(std::time(NULL)));
    SetChannelInfo(kTopicInfo,"");
    SetChannelInfo(kChannelPassword,"");
    SetChannelInfo(kTopicEditUser,"NONE");
    SetChannelInfo(kChannelUserLimit,itostr(kMaxChannelUsers));
    invited_users_.clear();
    users_in_channel_.clear();
    users_in_channel_[nickname] = kOperator;
	channel_mode_status_[kInvite] = false;
	channel_mode_status_[kTopic ] = true;
	channel_mode_status_[kPassword] = false;
	channel_mode_status_[kLimit] = false;
}

IRCChannel::IRCChannel(const std::string& nickname, const std::string& channel_name, const std::string& passwd)
    {
    SetChannelInfo(kChannelName,channel_name);
    SetChannelInfo(kChannelPassword,passwd);
    SetChannelInfo(kChannelDate,itostr(std::time(NULL)));
    SetChannelInfo(kTopicEditTime,itostr(std::time(NULL)));
    SetChannelInfo(kTopicInfo,"");
    SetChannelInfo(kTopicEditUser,"NONE");
    SetChannelInfo(kChannelUserLimit,itostr(kMaxChannelUsers));
    invited_users_.clear();
    users_in_channel_.clear();
    users_in_channel_[nickname] = kOperator;
	channel_mode_status_[kInvite] = false;
	channel_mode_status_[kTopic ] = true;
	channel_mode_status_[kPassword] = true;
	channel_mode_status_[kLimit] = false;
}

IRCChannel::~IRCChannel(){}

std::string IRCChannel::itostr(long long time) {
    std::stringstream result;
    result << time;
    return result.str();
}

void IRCChannel::DelChannelUser(const std::string& nickname){
    UserInChannel::iterator it = users_in_channel_.find(nickname);
    if(it == users_in_channel_.end())
        return;
    users_in_channel_.erase(it);
}

bool IRCChannel::IsUserAuthorized(const std::string& nickname, ChannelPermission option) {
    UserInChannel::iterator it = users_in_channel_.find(nickname);
    if (it == users_in_channel_.end() || it->second != option) {
        return false;
    }
    return true;
}

void IRCChannel::SetUserAuthorization(const std::string& nickname, ChannelPermission option) {
    UserInChannel::iterator it = users_in_channel_.find(nickname);
    if(it == users_in_channel_.end())
        return;
    it->second = option;
}
void IRCChannel::SetChannelInfo(ChannelInfo idx, const std::string& str){
    channel_info_arr_[idx] = str;
}

unsigned int IRCChannel::GetChannelUserSize() const{
    return users_in_channel_.size();
}

bool IRCChannel::IsInChannel(const std::string& nickname) const {
    if(users_in_channel_.find(nickname) != users_in_channel_.end())
        return true;
    return false;
}

bool IRCChannel::IsInvited(const std::string& nickname) const {
    return std::find(invited_users_.begin(), invited_users_.end(), nickname) != invited_users_.end();
}

void IRCChannel::AddChannelUser(const std::string& nickname) {
    users_in_channel_[nickname] =  kNormal;
}

void IRCChannel::ManageChannelPermission(const std::string& target_nickname, ChannelPermission option) {
    SetUserAuthorization(target_nickname, option);
}

std::string IRCChannel::GetChannelInfo(ChannelInfo idx)  const {
    return channel_info_arr_[idx];
}

void IRCChannel::AddInvitedUser(const std::string& target_nickname) {
    if(IsInvited(target_nickname))
        return;
    invited_users_.push_back(target_nickname);
}

void IRCChannel::DelInvitedUser(const std::string& target_nickname){
    InvitedUsers::iterator it = std::find(invited_users_.begin(),invited_users_.end(),target_nickname);
    if(it != invited_users_.end())
        invited_users_.erase(it);
}

std::deque<std::string> IRCChannel::GetChannelUsersWithPrefixes() const {
    std::deque<std::string> member_names;
	UserInChannel::const_iterator user;
    for (user = users_in_channel_.begin(); user != users_in_channel_.end(); ++user) {
        if (user->second == kOperator) {
            member_names.push_back("@" + user->first);
        } else {
            member_names.push_back(user->first);
        }
    }
    return member_names;
}

std::deque<std::string> IRCChannel::GetMemberNames() const {
    std::deque<std::string> member_names;
	UserInChannel::const_iterator user;
    for (user = users_in_channel_.begin(); user != users_in_channel_.end(); ++user) {
        member_names.push_back(user->first);
    }
    return member_names;
}

bool IRCChannel::CheckChannelMode(ChannelMode option) const {
	return channel_mode_status_[option];
}

void IRCChannel::SetChannelMode(ChannelMode option, bool flag) {
	channel_mode_status_[option] = flag;
}

std::string IRCChannel::GetChannelMode() const {
    std::string m;
	std::queue<std::string> m_info;
	if(CheckChannelMode(kInvite))
		m += "i";
	if(CheckChannelMode(kTopic))
		m += "t";
	if(CheckChannelMode(kPassword)) {
		m += "k";
		m_info.push(GetChannelInfo(kChannelPassword));
	}
	if(CheckChannelMode(kLimit)) {
		m += "l";
		m_info.push(GetChannelInfo(kChannelUserLimit));
	}
	while(!m_info.empty()) {
		m += ' ' + m_info.front();
		m_info.pop();
	}
	if(m.size() > 1)
		return "+" + m;
	return m;
}

bool IRCChannel::isValidChannelName(const std::string &name) {
	if(name.size() > 10 || name.size() < 1)
		return false;
	if(name[0] != '#')
		return false;
	if(std::string::npos != name.find('#',1))
		return false;
	for(unsigned int i = 1; i < name.size(); ++i)
	{
		if(!std::isalnum(static_cast<unsigned char>(name[i])))
			return false;
	}
	return true;
}