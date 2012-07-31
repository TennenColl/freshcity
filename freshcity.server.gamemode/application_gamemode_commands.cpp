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
#include "application_gamemode_colordefinitions.h"
#include "application_data_waypoint.h"
#include <sampgdk/a_players.h>
#include <sampgdk/a_vehicles.h>
#include "application_dependency_streamer.h"
#include "application_gamemode_object.h"
#include "application_algorithms.h"
#include "application_gamemode_sysmsgqueue.h"
#include "application_config.h"

class _CmdRegister {
public:
	_CmdRegister(const std::string& cmd, COMMAND_CALLBACK function, int reqlevel, unsigned int flags) {
		CommandManager::GetInstance().Add(cmd, function, reqlevel, flags);
	}
};

#define CMD(callback, cmd, levelreq, flags) \
	void Cmd##callback(Profile& player, const char* cmdline);\
	_CmdRegister __CmdReg##callback(cmd, Cmd##callback, levelreq, flags);\
	void Cmd##callback(Profile& player, const char* cmdline)

// Profile
CMD(SaveData, "sync", 0, NEED_SIGNED_IN) {
	player.Sync();
	player.SendChatMessage(COLOR_SUCC, "�����ѱ���");
}

// Attribute
CMD(SetSkin, "setskin", 0, NEED_SIGNED_IN) {
	int skinid(-1);
	if(sscanf(cmdline, "%d", &skinid) == 0 || skinid == -1) throw std::runtime_error("�÷�: /setskin <Ƥ��ID>");
	player.SetSkin(skinid);
	player.SendChatMessage(COLOR_SUCC, "Ƥ���Ѹ���");
}

// Weapon
CMD(GiveWeapon, "giveweapon", 1, NEED_SIGNED_IN) {
	int target(-1), weapon(-1), ammo(-1);
	if(sscanf(cmdline, "%d%d%d", &target, &weapon, &ammo) == 0
		|| !(target != -1 && weapon != -1 && ammo != -1))
		throw std::runtime_error("�÷�: /giveweapon <���ID> <����ID> <��ҩ��>");
	GivePlayerWeapon(target, weapon, ammo);
}

// Vehicle
CMD(GetVehicle, "v", 0, NULL) {
	int mid(-1);
	sscanf(cmdline, "%d", &mid);
	Coordinate3D playerpos = player.GetPos();
	int vid = CreateVehicle(mid, (float)playerpos.x, (float)playerpos.y, (float)playerpos.z + 0.5f,
		player.GetFacingAngle(), 1, 1, -1);
	LinkVehicleToInterior(vid, player.GetInterior());
	SetVehicleVirtualWorld(vid, player.GetVirtualWorld());
	PutPlayerInVehicle(player.GetId(), vid, 0);
}

// Teleporting
CMD(CreateWaypoint, "ctp", 0, NEED_SIGNED_IN) {
	if(cmdline[0] == 0)	throw std::runtime_error("�÷�: /ctp <���͵�����>");
	Waypoint create(player.GetDetailedPos());
	create.Create(cmdline, player.GetUniqueID());
	player.SendChatMessage(COLOR_SUCC, "�Ѵ������͵� " + std::string(cmdline));
}

CMD(UseWaypoint, "tp", 0, NULL) {
	if(cmdline[0] == 0)	throw std::runtime_error("�÷�: /tp <���͵�����>");
	Waypoint point(cmdline);
	point.PerformTeleport(player.GetId());
	player.SendChatMessage(COLOR_SUCC, "�Ѵ��͵� " + std::string(cmdline));
}

CMD(GoToPlayer, "goto", 0, NULL) {
	int targetid(-1);
	sscanf(cmdline, "%d", &targetid);
	if(!IsPlayerConnected(targetid)) throw std::runtime_error("�÷�: /goto <���ID>");
	Waypoint point(ProfileMgr[targetid].GetDetailedPos());
	point.PerformTeleport(player.GetId());
}

CMD(GetPlayer, "get", 1, NEED_SIGNED_IN) {
	int targetid(-1);
	sscanf(cmdline, "%d", &targetid);
	if(!IsPlayerConnected(targetid)) throw std::runtime_error("�÷�: /get <���ID>");
	Waypoint point(player.GetDetailedPos());
	point.PerformTeleport(targetid);
}

CMD(CreateTeleportTrigger, "ctpt", 0, NEED_SIGNED_IN) {
	Waypoint wp(cmdline);
	CreateTeleportTrigger(wp.GetUniqueID(), player.GetPos());
}

// Server Management
CMD(ReloadTeamList, "reloadteam", 65535, NEED_SIGNED_IN) {
	TeamMgr.LoadAllFromDatabase();
	player.SendChatMessage(COLOR_SUCC, "�����������Ŷ�����");
}

CMD(ReloadGangZoneList, "reloadgangzone", 65535, NEED_SIGNED_IN) {
	GangZoneMgr.LoadAllFromDatabase();
	player.SendChatMessage(COLOR_SUCC, "�����������������");
}

CMD(ReloadConfig, "reloadconfig", 65535, NEED_SIGNED_IN) {
	ReloadConfig();
	player.SendChatMessage(COLOR_SUCC, "�������������������");
}

// Team Management
CMD(CreateGangZone, "gzcreate", 65535, NEED_SIGNED_IN) {
	if(TeamMgr[player.GetTeamId()].GetLeader() != player.GetUniqueID())
		throw std::runtime_error("�㲻���Ŷ�����");
	if(player.GetVar<bool>("gz_create_in_progress"))
		throw std::runtime_error("�Ѿ��ڴ���������");
	if(cmdline[0] == 0)
		throw std::runtime_error("���Ʋ���Ϊ��");
	player.SetVar("gz_create_in_progress", true);
	player.SendChatMessage(COLOR_WARN, "�� ESC->MAP ���Ҽ����ñ����ȷ�� GangZone �ķ�Χ");
	player.SendChatMessage(COLOR_WARN, "���������ڵĵص㽫����Ϊ������, ���Ժ��� /gzsettrigger ȷ������ս���Ĵ������");
	player.SendChatMessage(COLOR_WARN, "/gzcreatedone ���洴���� GangZone, �� /gzcreatecancel ��ȡ����ǰ����");
	player.SetVar("gz_create_name", std::string(cmdline));
	player.SetVar("gz_create_spawnpoint", player.GetDetailedPos());
	player.GetVar<unsigned int>("gz_create_step") |= GANGZONE_CREATE_SPAWNPOINT;
}

CMD(CreateGangZoneTrigger, "gzsettrigger", 65535, NEED_SIGNED_IN) {
	if(!player.GetVar<bool>("gz_create_in_progress"))
		throw std::runtime_error("δ�ڴ���������");
	player.SetVar("gz_create_trigger", player.GetPos());
	player.SendChatMessage(COLOR_SUCC, "��ȷ������ս���������λ��");
	player.GetVar<unsigned int>("gz_create_step") |= GANGZONE_CREATE_TRIGGER;
}

CMD(CreateGangZoneDone, "gzcreatedone", 65535, NEED_SIGNED_IN) {
	if(!player.GetVar<bool>("gz_create_in_progress"))
		throw std::runtime_error("δ�ڴ���������");
	unsigned int step(player.GetVar<unsigned int>("gz_create_step"));
	if(step != 15) throw std::runtime_error("����δ��ɵ�����");
	std::string& name(player.GetVar<std::string>("gz_create_name"));
	Waypoint sp(player.GetVar<Coordinate5D>("gz_create_spawnpoint"));
	sp.Create("_System_GangZone_Spawnpoint_" + name, player.GetUniqueID());
	GangZoneManager::MemberPtr newgz(new GangZoneItem(
		player.GetVar<std::string>("gz_create_name"), player.GetTeamId(),
		player.GetVar<CoordinatePlane>("gz_create_pos_min"),
		player.GetVar<CoordinatePlane>("gz_create_pos_max"),
		sp.GetUniqueID(), player.GetVar<Coordinate3D>("gz_create_trigger")));
	GangZoneMgr.Add(newgz);
	player.SetVar("gz_create_in_progress", false);
	player.SetVar("gz_create_step", (unsigned)0);
	player.SendChatMessage(COLOR_SUCC, "���̴����ɹ�");
}

CMD(CreateGangZoneCancel, "gzcreatecancel", 65535, NEED_SIGNED_IN) {
	if(!player.GetVar<bool>("gz_create_in_progress"))
		throw std::runtime_error("δ�ڴ���������");
	player.SetVar("gz_create_in_progress", false);
	player.SetVar("gz_create_step", (unsigned)0);
	player.SendChatMessage(COLOR_SUCC, "��ȡ����������");
}

CMD(RemoveGangZone, "gzremove", 65535, NEED_SIGNED_IN) {
	std::stringstream content;
	MANAGER_FOREACH(GangZoneManager)
		content << iter->first << " " << iter->second->GetName() << "\n";
	DlgMgr.Show(DIALOG_GANGZONE_REMOVE, content.str(), player.GetId());
}
