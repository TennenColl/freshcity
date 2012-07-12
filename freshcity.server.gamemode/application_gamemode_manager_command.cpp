/*
 * Copyright 2012 TennenColl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "application_database.h"
#include "application_gamemode_manager_command.h"
#include "application_gamemode_manager_profile.h"

struct CommandManager::Callback {
	CallbackPtr ptr;
	int reqlevel;
	unsigned int flags;
	Callback(CallbackPtr ptr, int reqlevel, unsigned int flags) : ptr(ptr), reqlevel(reqlevel), flags(flags) {}
};

bool CommandManager::Add(const std::string& cmd, COMMAND_CALLBACK function, int reqlevel, unsigned int flags) {
	if(IsExist(cmd)) return false;
	_cmds.insert(std::make_pair(cmd, Callback(CallbackPtr(function), reqlevel, flags)));
	return true;
}

bool CommandManager::IsExist(const std::string& cmd) const {
	return _cmds.find(cmd) != _cmds.end();
}

bool CommandManager::Remove(const std::string& cmd) {
	CommandMap::const_iterator iter(_cmds.find(cmd));
	if(iter == _cmds.end()) return false;
	_cmds.erase(iter);
	return true;
}

#define MATCHREQ(req) ((iter->second.flags & req) == req)

void CommandManager::Exec(int playerid, const std::string& cmd, const char* cmdline) const {
	CommandMap::const_iterator iter(_cmds.find(cmd));
	if(iter == _cmds.end()) throw std::runtime_error("不存在的命令.");
	Profile& player = ProfileManager::GetInstance()[playerid];
	if(!MATCHREQ(NO_REQUIREMENT)) {
		if(MATCHREQ(NEED_REGISTERED) && ProfileManager::GetInstance()[playerid].IsRegistered() == false)
			throw std::runtime_error("此命令仅限已注册玩家使用.");
		if(MATCHREQ(NEED_SIGNED_IN) && ProfileManager::GetInstance()[playerid].IsSignedIn() == false)
			throw std::runtime_error("此命令仅限已登录玩家使用.");
		if(MATCHREQ(DONOT_REGISTERED) && ProfileManager::GetInstance()[playerid].IsRegistered() == true)
			throw std::runtime_error("此命令仅限未注册玩家使用.");
		if(MATCHREQ(DONOT_SIGNED_IN) && ProfileManager::GetInstance()[playerid].IsSignedIn() == true)
			throw std::runtime_error("此命令仅限未登录玩家使用.");
	}
	if(iter->second.reqlevel > player.GetAdminLevel())
		throw std::runtime_error("您没有足够管理权限来执行此命令.");
	iter->second.ptr(player, cmdline);
}

#undef MATCHREQ

CommandManager& CommandManager::GetInstance() {
	static CommandManager inst;
	return inst;
}
