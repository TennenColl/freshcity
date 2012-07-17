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
#include <sampgdk/a_players.h>
#include <sampgdk/a_samp.h>
#include <sampgdk/a_vehicles.h>
#include <sampgdk/plugin.h>
#include "basic_debug_logging.h"
#include "application_gamemode_manager_profile.h"
#include "application_gamemode_manager_command.h"
#include "application_gamemode_colordefinitions.h"
#include <boost/algorithm/string.hpp>
#include "basic_algorithm_random.h"
#include "application_gamemode_manager_team.h"

ProfileManager& ProfileMgr(ProfileManager::GetInstance());
TeamManager& TeamMgr(TeamManager::GetInstance());

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppPluginData) {
	sampgdk_initialize_plugin(ppPluginData);
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	return;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
	sampgdk_process_timers();
}

PLUGIN_EXPORT bool PLUGIN_CALL OnGameModeInit() {
	SetGameModeText("Freshcity");
	for(int i = 1; i < 299; i++) 
		AddPlayerClass(i, 1497.07f, -689.485f, 94.956f, 180.86f, 16, 3, 27, 100, 31, 100);
	TeamMgr.Add("Cops");
	TeamMgr["Cops"].SetColor(COLOR_BLUE);
	TeamMgr.Add("Criminals");
	TeamMgr["Criminals"].SetColor(COLOR_GREEN);
	LOG_INFO("Freshcity Gamemode ������");
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerConnect(int playerid) {
	try {
		SendClientMessage(playerid, COLOR_INFO, "��ӭ���� TennenColl �Ŀ���������");
		try {
			ProfileMgr.Add(playerid);
			ProfileMgr[playerid].SetSignedIn(false);
			Profile& player = ProfileMgr[playerid];
			if(!player.IsExistInDatabase()) {
				if(player.IsBannedForGame()) {
					player.SendChatMessage(COLOR_ERROR, "���Ѿ������������.");
					player.KickNow();
				} else {
					//player.SetColor(RandomRGBAColor());
					player.SendChatMessage(COLOR_INFO, "�㻹û��ע��, �� /register <����> ���������û�.");
				}
			} else {
				player.SendChatMessage(COLOR_WARN, "��ӭ����, " + player.GetName() + " . ��ִ�� /login <����> �Ե�¼.");
			}
			player.SetTeam(NO_TEAM);
			SendClientMessageToAll(COLOR_INFO, std::string(player.GetName() + " ���������.").c_str());
			player.SendChatMessage(COLOR_INFO, "/teamjoin Cops ���뾯��");
			player.SendChatMessage(COLOR_INFO, "/teamjoin Criminals �����ﷸ");
			player.SendChatMessage(COLOR_WARN, "��ע���Сд");
		} catch(std::runtime_error& e) {
			LOG_ERROR(e.what());
			throw;
		} catch(mongo::UserException& e) {
			LOG_ERROR(e.what());
			throw;
		} catch(...) {
			throw;
		}
	} catch(...) {
		SendClientMessage(playerid, COLOR_ERROR, "��ʼ���������ʱ��������, ����ϵ����������Ա.");
		return false;
	}
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason) {
	SendClientMessageToAll(COLOR_INFO, std::string(GetPlayerName(playerid) + " �뿪������.").c_str());
	try {
		if(reason != 0 /* timeout */ && ProfileMgr[playerid].IsSignedIn())
			ProfileMgr[playerid].Sync();
		int playerteamid = GetPlayerTeam(playerid);
		if(playerteamid != NO_TEAM)
			TeamMgr[TeamMgr.GetNameByID(playerteamid)].Quit(ProfileMgr[playerid]);
		ProfileMgr.Remove(playerid);
	} catch(...) {
		return false;
	}
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerRequestClass(int playerid, int classid) {
	SetPlayerPos(playerid, 1497.07f, -689.485f, 94.956f);
	SetPlayerFacingAngle(playerid, 180.86f);
	SetPlayerCameraPos(playerid, 1497.81f, -707.83f, 99.69f);
	SetPlayerCameraLookAt(playerid, 1493.39f, -686.97f, 98.35f, CAMERA_MOVE);
	return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerCommandText(int playerid, const char *cmdtext) {
	LOG_DEBUG(cmdtext);
	char cmdname[128] = {0}, cmdline[128] = {0};
	sscanf(cmdtext, "%s%*[ ]%[^\0]", cmdname, cmdline);
	try {
		CommandManager::GetInstance().Exec(playerid, boost::to_lower_copy(std::string(&cmdname[1])), cmdline);
		return true;
	} catch(std::runtime_error& e) {
		SendClientMessage(playerid, COLOR_ERROR, e.what());
		return true;
	} catch(...) {
		SendClientMessage(playerid, COLOR_ERROR, "��������ʱ����δ֪����, ����ϵ����������Ա.");
		return true;
	}
	return false;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerKeyStateChange(int playerid, int newkeys, int oldkeys) {
	if((newkeys & KEY_FIRE) == KEY_FIRE || (newkeys & KEY_ACTION) == KEY_ACTION)
		if(IsPlayerInAnyVehicle(playerid))
			AddVehicleComponent(GetPlayerVehicleID(playerid), 1010);
	return true;	
}
