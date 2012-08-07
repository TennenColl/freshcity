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
CMD(ServerMgmt, "s", 65535, NEED_SIGNED_IN) {
	DlgMgr.Show(DIALOG_SERVER_MAIN, "���������Ŷ��б�\n������������б�\n�����������������", player.GetId());
}

// Team Management
CMD(GangZoneMain, "gz", 65535, NEED_SIGNED_IN) {
	DlgMgr.Show(DIALOG_GANGZONE_MAIN, "����\nɾ��", player.GetId());
}

CMD(CreateGangZone, "gzc", 65535, NEED_SIGNED_IN) {
	if(!player.HasVar("gz_create"))
		throw std::runtime_error("δ�ڴ���������");
	DlgMgr.Show(DIALOG_GANGZONE_CREATE_PROCESS, "���浱ǰλ��Ϊ������\n���浱ǰλ��Ϊս��������\n��ɴ���\nȡ������", player.GetId());
}
