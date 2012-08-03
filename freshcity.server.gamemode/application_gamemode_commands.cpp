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
#include <sampgdk/a_vehicles.h>
#include "application_algorithms.h"
#include "application_gamemode_sysmsgqueue.h"
#include "application_data_waypoint.h"

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

CMD(SetNickName, "setnickname", 0, NEED_SIGNED_IN) {
	if(cmdline[0] == 0) throw std::runtime_error("Usage: /setnickname <Nickname>");
	player.SetNickname(cmdline);
	player.SendChatMessage(COLOR_SUCC, "�ǳƸ���Ϊ" + std::string(cmdline));
}

// Player-Self
CMD(SetSkin, "skin", 0, NEED_SIGNED_IN) {
	int skinid(-1);
	sscanf(cmdline, "%d", &skinid);
	if(skinid == -1) throw std::runtime_error("Usage: /skin <SkinID>");
	player.SetSkin(skinid);
	player.SendChatMessage(COLOR_SUCC, "Ƥ���Ѹ���");
}

CMD(GiveWeapon, "weapon", 0, NEED_SIGNED_IN) {
	int weapon(-1), ammo(-1);
	sscanf(cmdline, "%d%d", &weapon, &ammo);
	if(weapon == -1 || ammo == -1)
		throw std::runtime_error("Usage: /weapon <WeaponID> <Ammo>");
	GivePlayerWeapon(player.GetId(), weapon, ammo);
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
CMD(TeleportMain, "t", 0, NEED_SIGNED_IN) {
	if(cmdline[0] != 0) {
		Waypoint point(cmdline);
		point.PerformTeleport(player.GetId());
	} else {
		DlgMgr.Show(DIALOG_TELEPORT_MAIN, "�ڴ˴�����\n���͵�\n�鿴����\n�������ͱ��", player.GetId());
	}
}

CMD(GoToPlayer, "goto", 0, NULL) {
	int targetid(-1);
	sscanf(cmdline, "%d", &targetid);
	if(!IsPlayerConnected(targetid)) throw std::runtime_error("�÷�: /goto <�������ID>");
	Waypoint point(ProfileMgr[targetid].GetDetailedPos());
	point.PerformTeleport(player.GetId());
}

CMD(GetPlayer, "get", 1, NEED_SIGNED_IN) {
	int targetid(-1);
	sscanf(cmdline, "%d", &targetid);
	if(!IsPlayerConnected(targetid)) throw std::runtime_error("�÷�: /get <�������ID>");
	Waypoint point(player.GetDetailedPos());
	point.PerformTeleport(targetid);
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
	if(player.HasVar("gz_create"))
		throw std::runtime_error("�Ѿ��ڴ���������");
	if(cmdline[0] == 0)
		throw std::runtime_error("���Ʋ���Ϊ��");
	boost::shared_ptr<void> newgz(new GangZoneCreationInfo(cmdline));
	static_cast<GangZoneCreationInfo*>(newgz.get())->spawnpoint = player.GetDetailedPos();
	static_cast<GangZoneCreationInfo*>(newgz.get())->step |= GANGZONE_CREATE_SPAWNPOINT;
	player.SetVar("gz_create", newgz);
	player.SendChatMessage(COLOR_WARN, "�� ESC->MAP ���Ҽ����ñ����ȷ�� GangZone �ķ�Χ");
	player.SendChatMessage(COLOR_WARN, "���������ڵĵص㽫����Ϊ������, ���Ժ��� /gzsettrigger ȷ������ս���Ĵ������");
	player.SendChatMessage(COLOR_WARN, "/gzcreatedone ���洴���� GangZone, �� /gzcreatecancel ��ȡ����ǰ����");
}

CMD(CreateGangZoneTrigger, "gzsettrigger", 65535, NEED_SIGNED_IN) {
	if(!player.HasVar("gz_create"))
		throw std::runtime_error("δ�ڴ���������");
	GangZoneCreationInfo& data(player.GetVar<GangZoneCreationInfo>("gz_create"));
	data.trigger = player.GetPos();
	data.step |= GANGZONE_CREATE_TRIGGER;
	player.SendChatMessage(COLOR_SUCC, "��ȷ������ս���������λ��");
}

CMD(CreateGangZoneDone, "gzcreatedone", 65535, NEED_SIGNED_IN) {
	if(!player.HasVar("gz_create"))
		throw std::runtime_error("δ�ڴ���������");
	GangZoneCreationInfo& data(player.GetVar<GangZoneCreationInfo>("gz_create"));
	if(data.step != 15) throw std::runtime_error("����δ��ɵ�����");
	Waypoint sp(data.spawnpoint);
	sp.Create("_System_GangZone_Spawnpoint_" + mongo::OID().gen().str(), player.GetUniqueID());
	CoordinatePlane min(data.min.x > data.max.x ? data.max.x : data.min.x,
		data.min.y > data.max.y ? data.max.y : data.min.y),
		max(data.min.x < data.max.x ? data.max.x : data.min.x,
		data.min.y < data.max.y ? data.max.y : data.min.y);
	GangZoneManager::MemberPtr newgz(new GangZoneItem(
		data.name, player.GetTeamId(), min, max, sp.GetUniqueID(), data.trigger));
	GangZoneMgr.Add(newgz);
	player.DelVar("gz_create");
	player.SendChatMessage(COLOR_SUCC, "���̴����ɹ�");
}

CMD(CreateGangZoneCancel, "gzcreatecancel", 65535, NEED_SIGNED_IN) {
	if(!player.HasVar("gz_create"))
		throw std::runtime_error("δ�ڴ���������");
	player.DelVar("gz_create");
	player.SendChatMessage(COLOR_SUCC, "��ȡ����������");
}

CMD(RemoveGangZone, "gzremove", 65535, NEED_SIGNED_IN) {
	std::stringstream content;
	MANAGER_FOREACH(GangZoneManager)
		content << iter->first << "\t" << iter->second->GetName() << "\n";
	DlgMgr.Show(DIALOG_GANGZONE_REMOVE, content.str(), player.GetId());
}
