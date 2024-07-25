#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>

#include "IRCChannel.hpp"

IRCChannel::IRCChannel(const std::string& name, const IRCClient& client)
    : channel_name_(name) {
    start_date_ = SetTime();
	channel_topic_edit_date_ = SetTime(); 
    channel_limit_ = kMaxChannelUsers;
    channel_mode_ |= kDefault | kTopic;
    channel_topic_ = "";
    channel_password_ = "";
    invited_users_.clear();
    users_in_channel_.clear();
    users_in_channel_.insert(make_pair(client.GetNickname(), kOperator));
}

std::string IRCChannel::SetTime() {
    std::time_t time = std::time(nullptr);
    char time_string[20];
    std::strftime(time_string, 20, "%Y%m%d-%H:%M:%S", std::localtime(&time));
    return std::string(time_string);
}

void IRCChannel::AddChannelMode(const IRCClient& client, ChannelModeSet option) {
    if (!IsUserAuthorized(client, kOperator) || channel_mode_ & option) {
        return;
    }
    channel_mode_ |= option;
}

void IRCChannel::RemoveChannelMode(const IRCClient& client, ChannelModeSet option) {
    if (!IsUserAuthorized(client, kOperator)) {
        return;
    }
    if (channel_mode_ & option) {
        channel_mode_ &= ~option;
        if (option & kLimit) {
            channel_limit_ = kMaxChannelUsers;
        } else if (option & kPassword) {
            channel_password_ = "";
        }
    }
}

bool IRCChannel::CheckChannelMode(ChannelModeSet op) const {
    return channel_mode_ & op;
}

bool IRCChannel::IsUserAuthorized(const IRCClient& client, ChannelPermission option) {
    UserInChannel::iterator it = users_in_channel_.find(client.GetNickname());
    if (it == users_in_channel_.end()) {
        return false;
    }
    return it->second & option;
}

void IRCChannel::SetUserAuthorization(const IRCClient& client, ChannelPermission option) {
    if (!IsUserAuthorized(client, kOperator)) {
        return;  // No permission
    }
    UserInChannel::iterator it = users_in_channel_.find(client.GetNickname());
    if (it == users_in_channel_.end() || it->second & option) {
        return;  // User not in channel or already has the permission
    }
    it->second = option;
}

void IRCChannel::SetPassword(const IRCClient& client, const std::string& pass) {
    if (!IsUserAuthorized(client, kOperator)) {
        return;  // No permission
    }
    channel_password_ = pass;
}

void IRCChannel::SetTopic(const IRCClient& client, const std::string& topic) {
    if (CheckChannelMode(kTopic) && IsUserAuthorized(client, kOperator)) {
        channel_topic_ = topic;
		channel_topic_edit_date_ = SetTime();
    } else if (!CheckChannelMode(kTopic)) {
        channel_topic_ = topic;
		channel_topic_edit_date_ = SetTime();
    }
}

void IRCChannel::SetUserLimit(const IRCClient& client, const unsigned int& num) {
    AddChannelMode(client, kLimit);
    channel_limit_ = num;
}

void IRCChannel::AddInvitedUser(const IRCClient& client, const IRCClient& target) {
    if (IsInChannel(client) && !IsInvited(target)) {
        invited_users_.push_back(target.GetNickname());
    }
}

bool IRCChannel::IsInChannel(const IRCClient& client) const {
    return users_in_channel_.find(client.GetNickname()) != users_in_channel_.end();
}

bool IRCChannel::IsInvited(const IRCClient& client) const {
    return std::find(invited_users_.begin(), invited_users_.end(), client.GetNickname()) != invited_users_.end();
}

bool IRCChannel::MatchPassword(const std::string& password) const {
    return !CheckChannelMode(kPassword) || password == channel_password_;
}

void IRCChannel::AddChannelUser(const IRCClient& client, const IRCClient& target) {
    if (!IsInChannel(client)) {
        return;  // Inviter not in channel
    }
    if (CheckChannelMode(kLimit) && users_in_channel_.size() >= channel_limit_) {
        return;  // Channel limit exceeded
    }
    if (CheckChannelMode(kInvite) && !IsInvited(target)) {
        return;  // Invite-only mode and user not invited
    }
    if (IsInChannel(target)) {
        return;  // User already in channel
    }
    if (IsInvited(target)) {
    	InvitedUsers::iterator it = std::find(invited_users_.begin(), invited_users_.end(), target.GetNickname());
        invited_users_.erase(it);
    }
    users_in_channel_.insert(make_pair(target.GetNickname(), kNormal));
}

void IRCChannel::ManageChannelPermission(const IRCClient& client, const IRCClient& target, ChannelPermission option) {
    if (!IsUserAuthorized(client, kOperator)) {
        return;  // No permission
    }
    SetUserAuthorization(target, option);
}

std::string IRCChannel::GetChannelMode() const {
    std::string mode;
    if (channel_mode_ & kLimit) mode.push_back('l');
    if (channel_mode_ & kInvite) mode.push_back('i');
    if (channel_mode_ & kTopic) mode.push_back('t');
    if (channel_mode_ & kPassword) mode.push_back('k');
    if (channel_mode_ & kDefault) mode.push_back('s');
    return mode;
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