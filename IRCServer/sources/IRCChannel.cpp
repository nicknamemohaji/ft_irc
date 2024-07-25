#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>

#include "IRCChannel.hpp"

IRCChannel::IRCChannel(const std::string& nickname, const std::string& channel_name)
    : channel_name_(channel_name) {
    start_date_ = SetTime();
	channel_topic_edit_date_ = SetTime(); 
    channel_limit_ = kMaxChannelUsers;
    channel_mode_ |= kDefault | kTopic;
    channel_topic_ = "";
    channel_password_ = "";
    invited_users_.clear();
    users_in_channel_.clear();
    users_in_channel_.insert(make_pair(nickname, kOperator));
}

IRCChannel::IRCChannel(const std::string& nickname, const std::string& channel_name, const std::string& passwd)
    : channel_name_(channel_name), channel_password_(passwd) {
    start_date_ = SetTime();
	channel_topic_edit_date_ = SetTime(); 
    channel_limit_ = kMaxChannelUsers;
    channel_mode_ |= kDefault | kTopic;
    channel_topic_ = "";
    invited_users_.clear();
    users_in_channel_.clear();
    users_in_channel_.insert(make_pair(nickname, kOperator));
}

std::string IRCChannel::SetTime() {
    std::time_t time = std::time(nullptr);
    char time_string[20];
    std::strftime(time_string, 20, "%Y%m%d-%H:%M:%S", std::localtime(&time));
    return std::string(time_string);
}

bool IRCChannel::CheckChannelMode(ChannelModeSet op) const {
    return channel_mode_ & op;
}

bool IRCChannel::IsUserAuthorized(const std::string& nickname, ChannelPermission option) {
    UserInChannel::iterator it = users_in_channel_.find(nickname);
    if (it == users_in_channel_.end()) {
        return false;
    }
    return it->second & option;
}

void IRCChannel::SetUserAuthorization(const std::string& nickname, ChannelPermission option) {
    if (!IsUserAuthorized(nickname, kOperator)) {
        return;  // No permission
    }
    UserInChannel::iterator it = users_in_channel_.find(nickname);
    if (it == users_in_channel_.end() || it->second & option) {
        return;  // User not in channel or already has the permission
    }
    it->second = option;
}

void IRCChannel::SetTopic(const std::string& nickname, const std::string& topic) {
    if (CheckChannelMode(kTopic) && IsUserAuthorized(nickname, kOperator)) {
        channel_topic_ = topic;
		channel_topic_edit_date_ = SetTime();
    } else if (!CheckChannelMode(kTopic)) {
        channel_topic_ = topic;
		channel_topic_edit_date_ = SetTime();
    }
}

bool IRCChannel::IsInChannel(const std::string& nickname) const {
    return users_in_channel_.find(nickname) != users_in_channel_.end();
}

bool IRCChannel::IsInvited(const std::string& nickname) const {
    return std::find(invited_users_.begin(), invited_users_.end(), nickname) != invited_users_.end();
}

bool IRCChannel::MatchPassword(const std::string& password) const {
    return !CheckChannelMode(kPassword) || password == channel_password_;
}

void IRCChannel::AddChannelUser(const std::string& nickname, const std::string& target_nickname) {
    if (!IsInChannel(nickname)) {
        return;  // Inviter not in channel
    }
    if (CheckChannelMode(kLimit) && users_in_channel_.size() >= channel_limit_) {
        return;  // Channel limit exceeded
    }
    if (CheckChannelMode(kInvite) && !IsInvited(nickname)) {
        return;  // Invite-only mode and user not invited
    }
    if (IsInChannel(target_nickname)) {
        return;  // User already in channel
    }
    if (IsInvited(target_nickname)) {
    	InvitedUsers::iterator it = std::find(invited_users_.begin(), invited_users_.end(), target_nickname);
        invited_users_.erase(it);
    }
    users_in_channel_.insert(make_pair(target_nickname, kNormal));
}

void IRCChannel::ManageChannelPermission(const std::string& nickname, const std::string& target_nickname, ChannelPermission option) {
    if (!IsUserAuthorized(nickname, kOperator)) {
        return;  // No permission
    }
    SetUserAuthorization(target_nickname, option);
}

std::string IRCChannel::GetChannelStartTime() const {
    return start_date_;
}

std::string IRCChannel::GetTopic() const {
    return channel_topic_;
}

std::string IRCChannel::GetTopicEditDate() const {
    return channel_topic_edit_date_;
}
std::string IRCChannel::GetPassword() const {
    return channel_password_;
}

void IRCChannel::AddInvitedUser(const std::string& nickname, const std::string& target_nickname) {
    if (IsInChannel(nickname) && !IsInvited(target_nickname)) {
        invited_users_.push_back(target_nickname);
    }
}

std::vector<std::string> IRCChannel::GetMemberNames() const {
    std::vector<std::string> member_names;
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