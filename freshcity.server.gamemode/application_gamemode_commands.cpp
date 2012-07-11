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
#include "application_gamemode_colordefinitions.h"
#include <sampgdk/a_players.h>

#define CMD(x) void Cmd##x(Profile& player, const char* cmdline)

CMD(Register) {
	if(player.IsRegistered()) throw std::runtime_error("�����ע��.");
	if(cmdline[0] == 0) throw std::runtime_error("���벻��Ϊ��.");
	try {
		player.Create(player.GetName(), cmdline);
		player.SendChatMessage(COLOR_SUCC, std::string("ע��ɹ�. ��¼��: " + player.GetName() + ", ����(������): \"" + cmdline + "\".").c_str());
		ProfileManager::GetInstance().SetAuthed(player.GetId(), true);
	} catch(...) {
		throw std::runtime_error("ע��ʧ��.");
	}
}

CMD(Login) {
	if(!player.IsRegistered()) throw std::runtime_error("�����δע��.");
	if(ProfileManager::GetInstance().IsAuthed(player.GetId())) throw std::runtime_error("����ѵ�¼��.");
	if(!player.AuthPassword(cmdline)) throw std::runtime_error("�������.");
	ProfileManager::GetInstance().SetAuthed(player.GetId(), true);
	player.ApplyDataToPlayer();
	player.SendChatMessage(COLOR_SUCC, "��¼�ɹ�.");
}

CMD(LogOut) {
	if(!ProfileManager::GetInstance().IsAuthed(player.GetId())) throw std::runtime_error("�����δ��¼.");
	ProfileManager::GetInstance().SetAuthed(player.GetId(), false);
	player.SendChatMessage(COLOR_SUCC, "���ѵǳ�.");
}

CMD(SaveData) {
	if(!ProfileManager::GetInstance().IsAuthed(player.GetId())) throw std::runtime_error("���δ��¼.");
	player.Sync();
	player.SendChatMessage(COLOR_SUCC, "�����ѱ���.");
}

CMD(SetSkin) {
	int skinid(0);
	if(cmdline[0] == 0 || sscanf(cmdline, "%d", &skinid) == 0) throw std::runtime_error("�÷�: /setskin <Ƥ��ID>");
	player.SetSkin(skinid);
	player.SendChatMessage(COLOR_SUCC, "Ƥ���Ѹ���.");
}

CMD(GiveWeapon) {
	int target(-1), weapon(-1), ammo(-1);
	if(sscanf(cmdline, "%d%d%d", &target, &weapon, &ammo) == 0
		|| !(target != -1 && weapon != -1 && ammo != -1))
		throw std::runtime_error("�÷�: /giveweapon <���ID> <����ID> <��ҩ��>");
	GivePlayerWeapon(target, weapon, ammo);
}

#define REGCMD(x, y, z) CmdMgr.Add(x, y, z)

bool RegisterPlayerCmds() {
	CommandManager& CmdMgr = CommandManager::GetInstance();
	REGCMD("register",			CmdRegister,			0);
	REGCMD("login",				CmdLogin,				0);
	REGCMD("logout",			CmdLogOut,				0);
	REGCMD("sync",				CmdSaveData,			0);
	REGCMD("setskin",			CmdSetSkin,				1);
	REGCMD("giveweapon",		CmdGiveWeapon,			1);
	return true;
}

#undef REGCMD

void* PlayerCmdInit((void*)RegisterPlayerCmds());
