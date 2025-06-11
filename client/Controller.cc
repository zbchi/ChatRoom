#include "Controller.h"
#include "Client.h"
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <mutex>

#include <unistd.h>

State state_ = State::LOGINING;
void clearScreen()
{
    system("clear");
}

int getValidInt(const std::string &prompt)
{
    int value;
    while (true)
    {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "❌ 输入无效，请输入数字。\n";
        }
        else
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

void Controller::mainLoop()
{
    while (true)
    {
        switch (state_)
        {
        case State::REGISTERING:
            showRegister();
            break;
        case State::LOGINING:
            showLogin();
            break;
        case State::MAIN_MENU:
            showMainMenu();
            break;
        case State::CHAT_PANEL:
            showChatPanel();
            break;
        case State::SHOW_FREINDS:
            showFriends();
            break;
        case State::CHAT_FRIEND:
            chatWithFriend();
            break;
        case State::CHAT_GROUP:
            chatWithGroup();
            break;
        case State::ADD_FRIEND:
            showAddFriend();
            break;
        case State::DEL_FRIEND:
            showDelFriend();
            break;
        case State::HANDLE_FRIEND_REQUEST:
            showHandleFriendRequest();
            break;
        case State::CREATE_GROUP:
            showCreateGroup();
            break;
        case State::ADD_GROUP:
            showAddGroup();
            break;
        case State::HANDLE_GROUP_REQUEST:
            showHandleGroupRequest();
            break;
        case State::SHOW_GROUPS:
            showGroups();
            break;
        case State::SHOW_MEMBERS:
            showGroupMembers();
            break;
        case State::EXIT_GROUP:
            showExitGroup();
            break;

        case State::DESTORY_GROUP:
            showDestroyGroup();
            break;

        default:
            break;
        }
    }
}

void Controller::showMainMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        主菜单          ║
╚════════════════════════╝
[0] 📬 消息中心（聊天面板）
[1] 👤 好友相关功能
[2] 👥 群聊相关功能
[3] ⚙️ 系统设置/退出
)";
    int choice = getValidInt("请输入选项 (1-3): ");
    switch (choice)
    {
    case 0:
        state_ = State::CHAT_PANEL;
        break;
    case 1:
        showFriendMenu();
        break;
    case 2:
        showGroupMenu();
        break;
    case 3:
        showSystemMenu();
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showChatPanel()
{
    client_->friendService_.getFriends();
    client_->groupService_.getGroups();

    clearScreen();
    std::cout << MAGENTA << BOLD << R"(
┌═════════════════════════════════════┐
│            📬 消息中心              │
└═════════════════════════════════════┘
)" << RESET;

    std::vector<std::string> types;
    std::vector<std::string> ids;
    int index = 1;

    std::cout << YELLOW << "好友列表:\n"
              << RESET;
    for (const auto &f : client_->friendList_)
    {
        std::cout << BLUE << index << ". 👤 " << f.nickname_ << " "
                  << (f.isOnline_ ? GREEN "● 在线" : RED "○ 离线") << RESET << "\n";
        types.push_back("friend");
        ids.push_back(f.id_);
        ++index;
    }

    std::cout << YELLOW << "\n群聊列表:\n"
              << RESET;
    for (const auto &g : client_->groupList_)
    {
        std::cout << BLUE << index << ". 👥 " << g.group_name << RESET << "\n";
        types.push_back("group");
        ids.push_back(g.group_id_);
        ++index;
    }

    if (index == 1)
    {
        std::cout << YELLOW << "⚠️ 暂无好友或群聊，快去添加吧！\n"
                  << RESET;
    }

    std::cout << CYAN << "\n快捷操作:\n"
              << "[91] 好友请求 (" << client_->friendRequests_.size() << ")\n"
              << "[92] 群聊请求 (" << client_->groupAddRequests_.size() << ")\n"
              << "[0] 返回主菜单\n"
              << RESET;
    std::cout << GRADIENT_END << "➤ 选择编号或快捷键: " << RESET;

    int choice = getValidInt("");
    if (choice == 0)
    {
        state_ = State::MAIN_MENU;
    }
    else if (choice == 91)
    {
        state_ = State::HANDLE_FRIEND_REQUEST;
    }
    else if (choice == 92)
    {
        state_ = State::HANDLE_GROUP_REQUEST;
    }
    else if (choice >= 1 && choice < index)
    {
        if (types[choice - 1] == "friend")
        {
            for (auto &f : client_->friendList_)
            {
                if (f.id_ == ids[choice - 1])
                {
                    client_->currentFriend_.setCurrentFriend(f);
                    state_ = State::CHAT_FRIEND;
                    break;
                }
            }
        }
        else
        {
            for (auto &g : client_->groupList_)
            {
                if (g.group_id_ == ids[choice - 1])
                {
                    client_->currentGroup_.setCurrentGroup(g);
                    client_->groupService_.getGroupInfo();
                    state_ = State::CHAT_GROUP;
                    break;
                }
            }
        }
    }
    else
    {
        std::cout << RED << "⚠️ 无效选择\n"
                  << RESET;
    }
}

void Controller::showFriendMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║     👤 好友功能菜单     ║
╚════════════════════════╝

[1] 与好友聊天
[2] 添加好友
[3] 删除好友
[4] 处理好友请求
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::SHOW_FREINDS;
        break;
    case 2:
        state_ = State::ADD_FRIEND;
        break;
    case 3:
        state_ = State::DEL_FRIEND;
        break;
    case 4:
        state_ = State::HANDLE_FRIEND_REQUEST;
        break;
    case 0:
        state_ = State::MAIN_MENU;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showGroupMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║     👥 群聊功能菜单     ║
╚════════════════════════╝

[1] 创建群聊
[2] 加入群聊
[3] 处理加群申请
[4] 查看群聊列表
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::CREATE_GROUP;
        break;
    case 2:
        state_ = State::ADD_GROUP;
        break;
    case 3:
        state_ = State::HANDLE_GROUP_REQUEST;
        break;
    case 4:
        state_ = State::SHOW_GROUPS;
        break;
    case 0:
        state_ = State::MAIN_MENU;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showSystemMenu()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║      ⚙️ 系统设置菜单     ║
╚════════════════════════╝

[1] 退出登录
[0] 返回主菜单
)";
    int choice = getValidInt("请输入选项: ");
    switch (choice)
    {
    case 1:
        state_ = State::LOGINING;
        break;
    case 0:
        state_ = State::MAIN_MENU;
        break;
    default:
        std::cout << "❌ 无效选项\n";
        break;
    }
}

void Controller::showRegister()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       📝 用户注册       ║
╚════════════════════════╝
)";
    std::string email, password, nickname;
    std::cout << "📧 邮箱: ";
    std::cin >> email;
    std::cout << "🔐 密码: ";
    std::cin >> password;
    std::cout << "👤 昵称: ";
    std::cin >> nickname;

    client_->userService_.regiSter(email, password, nickname);

    while (true)
    {
        int code = getValidInt("📩 输入验证码: ");
        int reg_errno = client_->userService_.registerCode(email, password, nickname, code);
        if (reg_errno == 0)
        {
            std::cout << "✅ 注册成功!\n";
            state_ = State::LOGINING;
            break;
        }
        else
        {
            std::cout << "❌ 注册失败，错误码: " << reg_errno << "\n";
            if (reg_errno != 1)
            {
                state_ = State::REGISTERING;
                break;
            }
        }
    }
}

void Controller::showLogin()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║       🔑 用户登录       ║
╚════════════════════════╝
)";
    std::string email, password;
    std::cout << "📧 邮箱: ";
    std::cin >> email;

    while (true)
    {
        std::cout << "🔐 密码: ";
        std::cin >> password;
        int login_errno = client_->userService_.login(email, password);

        if (login_errno == 0)
        {
            std::cout << "✅ 登录成功，欢迎 " << client_->user_email_ << "\n";
            state_ = State::CHAT_PANEL;
            break;
        }
        else
        {
            std::cout << "❌ 登录失败，错误码: " << login_errno << "\n";
            if (login_errno != 1)
            {
                state_ = State::LOGINING;
                break;
            }
        }
    }
}

void Controller::showFriends()
{
    client_->friendService_.getFriends();
    flushFriends();
    int choice = getValidInt("");
    if (choice == 0)
    {
        state_ = State::MAIN_MENU;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }
    client_->currentFriend_.setCurrentFriend(client_->friendList_[choice - 1]);
    state_ = State::CHAT_FRIEND;
}

void Controller::chatWithFriend()
{
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
    clearScreen();
    std::cout << "💬 与好友聊天（输入 /exit 退出）\n";
    flushLogs();
    std::string content;
    while (true)
    {
        std::getline(std::cin, content);
        if (content.empty())
            continue;
        if (content == "/exit")
        {
            state_ = State::MAIN_MENU;
            break;
        }
        else if (content == "/c")
        {
            if (client_->chatLogs_[client_->currentFriend_.id_].empty())
            {
                std::cout << "没有更多聊天记录了" << std::endl;
                continue;
            }
            offset += count;
            client_->chatService_.loadMoreChatLogs(client_->currentFriend_.id_, count, offset);
            flushLogs();
            continue;
        }
        else if (content == "/ ")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentFriend_.id_, count, offset);
                flushLogs();
            }
            continue;
        }

        int chat_errno = client_->chatService_.sendMessage(content);
        if (chat_errno == 0)
        {
            client_->chatService_.loadInitChatLogs(client_->currentFriend_.id_, count);
            flushLogs();
        }
        else if (chat_errno == 1)
            std::cout << "❌发送失败(你们已不是好友)" << std::endl;
    }
}

void Controller::chatWithGroup()
{
    ssize_t offset = 0;
    int count = 20;
    client_->chatService_.loadInitChatLogs(client_->currentGroup_.group_id_, count, true);
    clearScreen();
    std::cout << "💬 群聊中（输入 /exit 退出）\n";
    flushGroupLogs();
    std::string content;
    while (true)
    {
        std::getline(std::cin, content);
        if (content.empty())
            continue;
        if (content == "/exit")
        {
            state_ = State::MAIN_MENU;
            break;
        }
        else if (content == "/c")
        {
            if (client_->groupChatLogs_[client_->currentGroup_.group_id_].empty())
            {
                std::cout << "没有更多聊天记录了" << std::endl;
                continue;
            }
            offset += count;
            client_->chatService_.loadMoreChatLogs(client_->currentGroup_.group_id_, count, offset, true);
            flushGroupLogs();
            continue;
        }
        else if (content == "/ ")
        {
            if (offset >= count)
            {
                offset -= count;
                client_->chatService_.loadMoreChatLogs(client_->currentGroup_.group_id_, count, offset, true);
                flushGroupLogs();
            }
            continue;
        }

        int chat_errno = client_->chatService_.sendGroupMessage(content);
        if (chat_errno == 0)
        {
            client_->chatService_.loadInitChatLogs(client_->currentGroup_.group_id_, count, true);
            flushGroupLogs();
        }
        else if (chat_errno == 1)
            std::cout << "❌发送失败(你已不在此群聊)" << std::endl;
    }
}

void Controller::showAddFriend()
{
    clearScreen();
    std::cout << "➕ 请输入要添加的好友ID: ";
    std::string friend_id;
    std::cin >> friend_id;
    client_->friendService_.addFriend(friend_id);
    state_ = State::MAIN_MENU;
}

void Controller::showDelFriend()
{
    clearScreen();
    if (client_->friendList_.empty())
    {
        std::cout << "⚠️ 当前没有好友。\n";
        return;
    }
    std::cout << "👥 好友列表:\n";
    for (size_t i = 0; i < client_->friendList_.size(); ++i)
        std::cout << i + 1 << ". " << client_->friendList_[i].nickname_ << "\n";
    int choice = getValidInt("🔢 选择要删除的好友编号: ");
    if (choice < 1 || choice > static_cast<int>(client_->friendList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }
    client_->friendService_.delFriend(client_->friendList_[choice - 1].id_);
    state_ = State::MAIN_MENU;
}

void Controller::showCreateGroup()
{
    clearScreen();
    std::string name, desc;
    std::cout << "📛 群名: ";
    std::cin >> name;
    std::cout << "📝 群描述: ";
    std::cin >> desc;
    client_->groupService_.createGroup(name, desc);
    state_ = State::MAIN_MENU;
}

void Controller::showAddGroup()
{
    clearScreen();
    std::string gid;
    std::cout << "🔗 输入要加入的群ID: ";
    std::cin >> gid;
    client_->groupService_.addGroup(gid);
    state_ = State::MAIN_MENU;
}
void Controller::showHandleFriendRequest()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(client_->friendService_.friendRequests_mutex_);
            flushRequests();
        }

        int i = getValidInt("🔢 选择请求编号 (0 返回): ");
        if (i == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->friendRequests_.size()))
        {
            std::cout << "❌ 无效编号\n";
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }
        else if (action == 1)
        {
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "accept");
        }
        else if (action == 2)
        {
            client_->friendService_.responseFriendRequest(client_->friendRequests_[i - 1], "reject");
        }
        else
        {
            std::cout << "❌ 无效操作\n";
        }
    }
}

void Controller::showHandleGroupRequest()
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(client_->groupService_.groupAddRequests_mutex_);
            flushGroupRequests();
        }

        int i = getValidInt("🔢 选择请求编号 (0 返回): ");
        if (i == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }

        if (i < 1 || i > static_cast<int>(client_->groupAddRequests_.size()))
        {
            std::cout << "❌ 无效编号\n";
            continue;
        }

        std::cout << "1. ✅ 接受\n2. ❌ 拒绝\n";
        int action = getValidInt("请选择操作: ");
        if (action == 0)
        {
            state_ = State::MAIN_MENU;
            return;
        }
        else if (action == 1)
        {
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "accept");
        }
        else if (action == 2)
        {
            client_->groupService_.responseGroupRequest(client_->groupAddRequests_[i - 1], "reject");
        }
        else
        {
            std::cout << "❌ 无效操作\n";
        }
    }
}

void Controller::showGroups()
{
    client_->groupService_.getGroups();
    flushGroups();
    int choice = getValidInt("");
    if (choice == 0)
    {
        state_ = State::MAIN_MENU;
        return;
    }
    if (choice < 1 || choice > static_cast<int>(client_->groupList_.size()))
    {
        std::cout << "❌ 无效编号\n";
        return;
    }

    client_->currentGroup_.setCurrentGroup(client_->groupList_[choice - 1]);
    client_->groupService_.getGroupInfo();
    state_ = State::SHOW_MEMBERS;
}
void Controller::showExitGroup()
{
    clearScreen();
    std::cout << "⚠️ 确认退出当前群聊？(1=是): ";
    int ch = getValidInt("");
    if (ch == 1)
        client_->groupService_.exitGroup();
    state_ = State::MAIN_MENU;
}

void Controller::showGroupMembers()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════════════╗
║        👥 群成员列表           ║
╚════════════════════════════════╝
)";
    std::vector<std::string> member_ids;
    std::vector<std::string> roles;
    int i = 0;
    for (const auto &pair : client_->currentGroup_.group_members)
    {
        std::cout << i + 1 << ". 👤 " << pair.second.nickname_
                  << " | 🏷 角色: " << pair.second.role_
                  << " | 🆔: " << pair.second.id_ << "\n";
        member_ids.push_back(pair.second.id_);
        roles.push_back(pair.second.role_);
        ++i;
    }

    if (member_ids.empty())
    {
        std::cout << "⚠️ 没有成员\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    int choice = getValidInt("🔢 选择成员编号进行管理 (0 返回): ");
    if (choice == 0)
    {
        state_ = State::SHOW_GROUPS;
        return;
    }

    if (choice < 1 || choice > static_cast<int>(member_ids.size()))
    {
        std::cout << "❌ 无效编号\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    std::string target_id = member_ids[choice - 1];
    std::string target_role = roles[choice - 1];
    std::string my_role = client_->currentGroup_.group_members[client_->user_id_].role_;

    // 群主 or 管理员才有权限管理他人
    if (my_role == "member")
    {
        std::cout << "🚫 你没有管理权限。\n";
        state_ = State::SHOW_GROUPS;
        return;
    }

    std::cout << R"(
选择操作:
1. ❌ 踢出成员
2. ⬆️ 设为管理员
3. ⬇️ 取消管理员
0. 返回
)";
    int action = getValidInt("输入操作编号: ");
    switch (action)
    {
    case 0:
        break;
    case 1:
        client_->groupService_.kickMember(target_id);
        std::cout << "✅ 已踢出成员。\n";
        break;
    case 2:
        if (target_role == "admin" || target_role == "owner")
        {
            std::cout << "⚠️ 对方已经是管理员或群主。\n";
        }
        else
        {
            client_->groupService_.addAdmin(target_id);
            std::cout << "✅ 已设为管理员。\n";
        }
        break;
    case 3:
        if (target_role != "admin")
        {
            std::cout << "⚠️ 对方不是管理员，无法取消。\n";
        }
        else
        {
            client_->groupService_.removeAdmin(target_id);
            std::cout << "✅ 已取消管理员。\n";
        }
        break;
    default:
        std::cout << "❌ 无效操作。\n";
        break;
    }

    state_ = State::SHOW_GROUPS;
}

void Controller::showDestroyGroup()
{
    clearScreen();
    bool isOwner = client_->currentGroup_.group_members[client_->user_id_].role_ == "owner";
    if (isOwner)
    {
        std::cout << "⚠️ 你是群主，此操作将解散群聊！\n";
    }
    else
    {
        std::cout << "⚠️ 你将退出该群聊。\n";
    }
    int confirm = getValidInt("确认操作？(1=是): ");
    if (confirm == 1)
    {
        client_->groupService_.exitGroup();
        std::cout << "✅ 操作已完成。\n";
    }
    state_ = State::MAIN_MENU;
}

void Controller::flushLogs()
{
    clearScreen();
    std::cout << MAGENTA << BOLD << R"(
┌─────────────────────────────────────────────┐
│ 与 )" << client_->currentFriend_.nickname_
              << " 的聊天" << std::string(30 - client_->currentFriend_.nickname_.length(), ' ') << R"(│
└─────────────────────────────────────────────┘
)" << RESET;

    for (const auto &log : client_->chatLogs_[client_->currentFriend_.id_])
    {
        std::string time = log.timestamp;
        std::string sender = log.sender_id == client_->user_id_ ? "我" : client_->currentFriend_.nickname_;
        std::string content = log.content;

        if (log.sender_id == client_->user_id_)
        {
            std::cout << GREEN << "┌────────────────────────────────────────────────────────┐\n"
                      << "│ " << std::left << std::setw(10) << sender << " " << std::right << std::setw(30) << time << " \n"
                      << "│ " << std::left << std::setw(40) << content << " \n"
                      << "└────────────────────────────────────────────────────────┘\n"
                      << RESET;
        }
        else
        {
            std::cout << "┌────────────────────────────────────────────────────────┐\n"
                      << "│ " << std::left << std::setw(10) << sender << " " << std::right << std::setw(30) << time << " \n"
                      << "│ " << std::left << std::setw(40) << content << " \n"
                      << "└────────────────────────────────────────────────────────┘\n";
        }
    }
}

void Controller::flushGroupLogs()
{
    clearScreen();
    std::cout << MAGENTA << BOLD << R"(
┌─────────────────────────────────────────────┐
│ 群聊: )" << client_->currentGroup_.group_name
              << std::string(30 - client_->currentGroup_.group_name.length(), ' ') << R"(│
└─────────────────────────────────────────────┘
)" << RESET;

    for (const auto &log : client_->groupChatLogs_[client_->currentGroup_.group_id_])
    {
        std::string time = log.timestamp;
        std::string sender = log.sender_id == client_->user_id_ ? "我" : client_->currentGroup_.group_members[log.sender_id].nickname_;
        std::string content = log.content;

        if (log.sender_id == client_->user_id_)
        {
            std::cout << GREEN << "┌────────────────────────────────────────────────────────┐\n"
                      << "│ " << std::left << std::setw(10) << sender << " " << std::right << std::setw(30) << time << " \n"
                      << "│ " << std::left << std::setw(40) << content << " \n"
                      << "└────────────────────────────────────────────────────────┘\n"
                      << RESET;
        }
        else
        {
            std::cout << "┌─────────────────────────────────────────────┐\n"
                      << "│ " << std::left << std::setw(10) << sender << " " << std::right << std::setw(30) << time << " \n"
                      << "│ " << std::left << std::setw(40) << content << " \n"
                      << "└─────────────────────────────────────────────┘\n";
        }
    }
}
void Controller::flushFriends()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        好友列表        ║
╚════════════════════════╝
)";
    if (client_->friendList_.empty())
    {
        std::cout << "⚠️ 没有好友。\n";
        return;
    }

    for (size_t i = 0; i < client_->friendList_.size(); ++i)
    {
        std::cout << (i + 1) << ". " << client_->friendList_[i].nickname_
                  << " [" << (client_->friendList_[i].isOnline_ ? "🟢 在线" : "🔴 离线") << "]\n";
    }
    std::cout << "🔢 请输入要选择的好友编号 (或 0 返回): ";
}

void Controller::flushRequests()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║      好友请求列表      ║
╚════════════════════════╝
)";
    int i = 1;
    for (const auto &req : client_->friendRequests_)
    {
        std::cout << i << ". 👤 昵称: " << req.nickname_
                  << " | 🆔: " << req.from_user_id
                  << " | 🕒 时间: " << req.timestamp_ << "\n";
        ++i;
    }
}

void Controller::flushGroupRequests()
{
    clearScreen();
    std::cout << R"(
╔══════════════════════════════╗
║        群聊加群请求列表      ║
╚══════════════════════════════╝
)";
    int i = 1;
    for (const auto &req : client_->groupAddRequests_)
    {
        std::cout << i << ". 📛 群: " << req.group_name
                  << " | 👤 用户: " << req.nickname
                  << " | 🕒 时间: " << req.timestamp << "\n";
        ++i;
    }
}

void Controller::flushGroups()
{
    clearScreen();
    std::cout << R"(
╔════════════════════════╗
║        群聊列表        ║
╚════════════════════════╝
)";
    if (client_->groupList_.empty())
    {
        std::cout << "⚠️ 当前没有加入任何群聊。\n";
        return;
    }

    for (size_t i = 0; i < client_->groupList_.size(); ++i)
    {
        std::cout << (i + 1) << ". 📛 " << client_->groupList_[i].group_name << "\n";
    }

    std::cout << "🔢 请输入要选择的群聊编号 (或 0 返回): ";
}