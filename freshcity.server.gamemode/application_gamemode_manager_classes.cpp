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
#include "application_gamemode_manager_classes.h"
#include "application_config.h"

// ProfileManager
bool ProfileManager::Add(int playerid) {
	return ItemManager::Add(playerid, MemberPtr(new Profile(playerid, GetPlayerName(playerid))));
}

// CommandManager
bool CommandManager::Add(const std::string& cmd, COMMAND_CALLBACK function, int reqlevel, unsigned int flags) {
	return ItemManager::Add(cmd, MemberPtr(new CommandCallbackCell(CommandPtr(function), reqlevel, flags)));
}

#define MATCHREQ(req) ((iter->second->flags & req) == req)

void CommandManager::Exec(int playerid, const std::string& cmd, const char* cmdline) {
	MemberMap::const_iterator iter(_members.find(cmd));
	if(iter == _members.end()) throw std::runtime_error("�����ڵ�����");
	Profile& player = ProfileManager::GetInstance()[playerid];
	if(!MATCHREQ(NO_REQUIREMENT)) {
		if(MATCHREQ(NEED_REGISTERED) && ProfileManager::GetInstance()[playerid].IsEmpty())
			throw std::runtime_error("�����������ע�����ʹ��");
		if(MATCHREQ(NEED_SIGNED_IN) && !ProfileManager::GetInstance()[playerid].IsSignedIn())
			throw std::runtime_error("����������ѵ�¼���ʹ��");
		if(MATCHREQ(DONOT_REGISTERED) && !ProfileManager::GetInstance()[playerid].IsEmpty())
			throw std::runtime_error("���������δע�����ʹ��");
		if(MATCHREQ(DONOT_SIGNED_IN) && ProfileManager::GetInstance()[playerid].IsSignedIn())
			throw std::runtime_error("���������δ��¼���ʹ��");
	}
	if(iter->second->reqlevel > player.GetAdminLevel())
		throw std::runtime_error("��û���㹻����Ȩ����ִ�д�����");
	iter->second->ptr(player, cmdline);
}

#undef MATCHREQ

// DialogManager
bool DialogManager::Add(int dialogid, DIALOG_CALLBACK function) {
	return ItemManager::Add(dialogid, MemberPtr(new DialogPtr(function)));
}

void DialogManager::Exec(int playerid, bool response, int dialogid, int listitem, const char* inputtext) {
	MemberMap::const_iterator iter(_members.find(dialogid));
	if(iter == _members.end()) throw std::runtime_error("δע��ص��ĶԻ���");
	Profile& player = ProfileManager::GetInstance()[playerid];
	iter->second->operator()(player, response, listitem, inputtext);
}

// EffectiveItemManager
bool EffectiveItemManager::Add(MemberPtr& item) {
	return ItemManager::Add(item->GetID(), item);
}

void EffectiveItemManager::Exec(int playerid, int itemid) {
	MemberMap::const_iterator iter(_members.find(itemid));
	if(iter == _members.end()) throw std::runtime_error("��Ч����ID");
	Profile& player = ProfileManager::GetInstance()[playerid];
	iter->second->Effect(player);
	if(iter->second->IsDisposable()) _members.erase(iter);
}

// TeamManager
TeamManager::TeamManager() : _teamidcounter(0) {}

bool TeamManager::Add(const std::string& teamname) {
	MemberPtr ptr(new Team(teamname, ++_teamidcounter));
	if(ItemManager::Add(teamname, ptr)) {
		_idtoname.insert(std::make_pair(_teamidcounter, teamname));
		return true;
	} else return false;
}

bool TeamManager::Remove(const std::string& teamname) {
	int tid = ItemManager::Get(teamname).GetIngameID();
	if(ItemManager::Remove(teamname)) {
		_idtoname.erase(_idtoname.find(tid));
		return true;
	} return false;
}

std::string TeamManager::GetNameByID(int teamid) const {
	IDNameMap::const_iterator iter = _idtoname.find(teamid);
	if(iter == _idtoname.end())
		throw std::runtime_error("�޷����ָ���Ŷӵ�ID");
	return iter->second;
}

// GangZoneManager
bool GangZoneManager::Add(MemberPtr& item) {
	return ItemManager::Add(item->Get().GetId(), item);
}

void GangZoneManager::LoadAllFromDatabase() {
	std::auto_ptr<mongo::DBClientCursor> _cursor
		= GetDB().query(CONFIG_STRING("Database.gangzone"), mongo::BSONObj());
	_members.clear();
	while(_cursor->more()) {
		MemberPtr _item(new GangZoneItem(_cursor->next()));
		ItemManager::Add(_item->Get().GetId(), _item);
	}
}
