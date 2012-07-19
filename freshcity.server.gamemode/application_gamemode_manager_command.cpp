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

bool CommandManager::Add(const std::string& cmd, COMMAND_CALLBACK function, int reqlevel, unsigned int flags) {
	return ItemManager::Add(cmd, MemberPtr(new CommandCallbackCell(CommandPtr(function), reqlevel, flags)));
}

#define MATCHREQ(req) ((iter->second->flags & req) == req)

void CommandManager::Exec(int playerid, const std::string& cmd, const char* cmdline) {
	MemberMap::const_iterator iter(_members.find(cmd));
	if(iter == _members.end()) throw std::runtime_error("�����ڵ�����");
	Profile& player = ProfileManager::GetInstance()[playerid];
	if(!MATCHREQ(NO_REQUIREMENT)) {
		if(MATCHREQ(NEED_REGISTERED) && ProfileManager::GetInstance()[playerid].IsExistInDatabase() == false)
			throw std::runtime_error("�����������ע�����ʹ��");
		if(MATCHREQ(NEED_SIGNED_IN) && ProfileManager::GetInstance()[playerid].IsSignedIn() == false)
			throw std::runtime_error("����������ѵ�¼���ʹ��");
		if(MATCHREQ(DONOT_REGISTERED) && ProfileManager::GetInstance()[playerid].IsExistInDatabase() == true)
			throw std::runtime_error("���������δע�����ʹ��");
		if(MATCHREQ(DONOT_SIGNED_IN) && ProfileManager::GetInstance()[playerid].IsSignedIn() == true)
			throw std::runtime_error("���������δ��¼���ʹ��");
	}
	if(iter->second->reqlevel > player.GetAdminLevel())
		throw std::runtime_error("��û���㹻����Ȩ����ִ�д�����");
	iter->second->ptr(player, cmdline);
}

#undef MATCHREQ
