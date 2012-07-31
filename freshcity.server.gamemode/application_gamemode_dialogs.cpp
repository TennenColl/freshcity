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
#include "application_data_waypoint.h"
#include "application_gamemode_colordefinitions.h"
#include "application_gamemode_role_classes.h"

class _DialogRegister {
public:
	_DialogRegister(int dialogid, const DialogCell& dlginfo) {
		DialogManager::GetInstance().Add(dialogid, dlginfo);
	}
};

#define DIALOG(id, style, caption, btnOK, btnCancle) \
	void Dlg##id(Profile& player, bool response, int listitem, const char* inputtext);\
	_DialogRegister __DlgReg_##id(id, DialogCell(style, caption, btnOK, btnCancle, Dlg##id));\
	void Dlg##id(Profile& player, bool response, int listitem, const char* inputtext)

DIALOG(DIALOG_TEAM_SELECT, DIALOG_STYLE_LIST, "ѡ����Ӫ", "ȷ��", "") {
	std::string id(inputtext, 24);
	mongo::OID ptid(player.GetTeamId());
	if(ptid != mongo::OID(id)) {
		if(player.GetTeamId().isSet())
			TeamMgr[ptid].Quit(player);
		TeamMgr[mongo::OID(id)].Join(player);
	}
	DlgMgr.Show(DIALOG_ROLE_SELECT, "Assault\nMedic\nMechanic\nEngineer", player.GetId());
}

DIALOG(DIALOG_ROLE_SELECT, DIALOG_STYLE_LIST, "ѡ��ְҵ", "ȷ��", "") {
	switch(listitem) {
	case 0:
		player.SetRole(Profile::RolePtr(new Assault(player)));
		break;

	case 1:
		player.SetRole(Profile::RolePtr(new Medic(player)));
		break;

	case 2:
		player.SetRole(Profile::RolePtr(new Mechanic(player)));
		break;

	case 3:
		player.SetRole(Profile::RolePtr(new Engineer(player)));
		break;
	}
	player.Spawn();
}

DIALOG(DIALOG_PROFILE_REGISTER, DIALOG_STYLE_PASSWORD, "ע��", "ע��", "") {
	if(inputtext[0] == 0) {
		player.SendChatMessage(COLOR_ERROR, "���벻��Ϊ��");
	} else {
		try {
			player.Create(player.GetName(), inputtext);
			player.SendChatMessage(COLOR_SUCC, "ע��ɹ�");
			player.SetSignedIn(true);
			return;
		} catch(std::runtime_error& e) {
			player.SendChatMessage(COLOR_ERROR, e.what());
		} catch(...) {
			player.SendChatMessage(COLOR_ERROR, "����δ֪ԭ��ע��ʧ��, ����ϵ����������Ա");
		}
	}
	DlgMgr.Show(DIALOG_PROFILE_REGISTER, "��������������:", player.GetId());
}

DIALOG(DIALOG_PROFILE_LOGIN, DIALOG_STYLE_PASSWORD, "��¼", "��¼", "") {
	if(!player.AuthPassword(inputtext)) {
		DlgMgr.Show(DIALOG_PROFILE_LOGIN, "��������������:", player.GetId());
		throw std::runtime_error("�������");
	}
	player.SetSignedIn(true);
	player.ApplyDataToPlayer();
	player.SendChatMessage(COLOR_SUCC, "��¼�ɹ�");
}

DIALOG(DIALOG_GANGZONE_CHOOSETOSPAWN, DIALOG_STYLE_LIST, "ѡ���������", "ȷ��", "") {
	int zoneid;
	sscanf(inputtext, "%d", &zoneid);
	Waypoint sp(GangZoneMgr[zoneid].GetSpawnPoint());
	sp.PerformTeleport(player.GetId());
}

DIALOG(DIALOG_GANGZONE_REMOVE, DIALOG_STYLE_LIST, "ѡ����Ҫɾ���� GangZone", "ɾ��", "ȡ��") {
	if(!response) return;
	int zoneid;
	sscanf(inputtext, "%d", &zoneid);
	GangZoneMgr[zoneid].Destroy();
	GangZoneMgr.Remove(zoneid);
	player.SendChatMessage(COLOR_SUCC, "ָ�� GangZone �������Ƴ�");
}
