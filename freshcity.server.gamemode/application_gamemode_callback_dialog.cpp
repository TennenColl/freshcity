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
#include "application_gamemode_role_classes.h"
#include "application_data_waypoint.h"

class _DialogRegister {
public:
	_DialogRegister(int dialogid, const DialogCell& dlginfo) {
		DialogManager::GetInstance().Add(dialogid, dlginfo);
	}
};

#define DIALOG(id, style, caption, btnOK, btnCancle, mustresponse) \
	void Dlg##id(Profile& player, bool response, int listitem, const char* inputtext);\
	_DialogRegister __DlgReg_##id(id, DialogCell(style, caption, btnOK, btnCancle, Dlg##id, mustresponse));\
	void Dlg##id(Profile& player, bool response, int listitem, const char* inputtext)

DIALOG(DIALOG_TEAM_SELECT, DIALOG_STYLE_LIST, "ѡ����Ӫ", "ȷ��", "", true) {
	std::string id(inputtext, 24);
	mongo::OID ptid(player.GetTeamId());
	if(ptid != mongo::OID(id)) {
		if(player.GetTeamId().isSet())
			TeamMgr[ptid].Quit(player);
		TeamMgr[mongo::OID(id)].Join(player);
	}
	DlgMgr.Show(DIALOG_ROLE_SELECT, "Assault\nMedic\nMechanic\nEngineer", player.GetId());
}

DIALOG(DIALOG_ROLE_SELECT, DIALOG_STYLE_LIST, "ѡ��ְҵ", "ȷ��", "", true) {
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

DIALOG(DIALOG_PROFILE_REGISTER, DIALOG_STYLE_PASSWORD, "ע��", "ע��", "", true) {
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
	DlgMgr.Show(DIALOG_PROFILE_REGISTER, "��������������", player.GetId());
}

DIALOG(DIALOG_PROFILE_LOGIN, DIALOG_STYLE_PASSWORD, "��¼", "��¼", "", true) {
	if(!player.AuthPassword(inputtext)) {
		DlgMgr.Show(DIALOG_PROFILE_LOGIN, "��������������", player.GetId());
		throw std::runtime_error("�������");
	}
	player.SetSignedIn(true);
	player.ApplyDataToPlayer();
	player.SendChatMessage(COLOR_SUCC, "��¼�ɹ�");
}

DIALOG(DIALOG_GANGZONE_CHOOSETOSPAWN, DIALOG_STYLE_LIST, "ѡ���������", "ȷ��", "", true) {
	int zoneid;
	sscanf(inputtext, "%d", &zoneid);
	Waypoint sp(GangZoneMgr[zoneid].GetSpawnPoint());
	sp.PerformTeleport(player.GetId());
}

DIALOG(DIALOG_GANGZONE_MAIN, DIALOG_STYLE_LIST, "GangZone", "ȷ��", "ȡ��", false) {
	switch(listitem) {
	case 0: /* Create */ {
		if(TeamMgr[player.GetTeamId()].GetLeader() != player.GetUniqueID())
			throw std::runtime_error("�㲻���Ŷ�����");
		if(player.HasVar("gz_create"))
			throw std::runtime_error("�Ѿ��ڴ���������");
		DlgMgr.Show(DIALOG_GANGZONE_CREATE, "ָ��һ������", player.GetId());
		break;
	}

	case 1: /* Remove */ {
		std::stringstream content;
		MANAGER_FOREACH(GangZoneManager)
			content << iter->first << "\t" << iter->second->GetName() << "\n";
		DlgMgr.Show(DIALOG_GANGZONE_REMOVE, content.str(), player.GetId());
		break;
	}

	default:
		break;
	}
}

DIALOG(DIALOG_GANGZONE_CREATE, DIALOG_STYLE_INPUT, "���� GangZone", "ȷ��", "ȡ��", false) {
	if(inputtext[0] == 0)
		throw std::runtime_error("���Ʋ���Ϊ��");
	boost::shared_ptr<void> newgz(new GangZoneCreationInfo(inputtext));
	player.SetVar("gz_create", newgz);
	player.SendChatMessage(COLOR_WARN, "�� ESC->MAP ���Ҽ����ñ����ȷ�� GangZone �ķ�Χ");
	player.SendChatMessage(COLOR_WARN, "ʹ�� /gzc ����ɺ�������");
}

DIALOG(DIALOG_GANGZONE_CREATE_PROCESS, DIALOG_STYLE_LIST, "���� GangZone", "ȷ��", "ȡ��", false) {
	GangZoneCreationInfo& data(player.GetVar<GangZoneCreationInfo>("gz_create"));
	switch(listitem) {
	case 0: /* Spwanpoint */
		data.spawnpoint = player.GetDetailedPos();
		data.step |= GANGZONE_CREATE_SPAWNPOINT;
		break;

	case 1: /* TurfwarTrigger */
		data.trigger = player.GetPos();
		data.step |= GANGZONE_CREATE_TRIGGER;
		break;

	case 2: /* Finish */ {
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
		break;
	}

	case 3: /* Cancel */
		player.DelVar("gz_create");
		break;
	}
}

DIALOG(DIALOG_GANGZONE_REMOVE, DIALOG_STYLE_LIST, "ѡ����Ҫɾ���� GangZone", "ɾ��", "ȡ��", false) {
	int zoneid;
	sscanf(inputtext, "%d", &zoneid);
	GangZoneMgr[zoneid].Destroy();
	GangZoneMgr.Remove(zoneid);
	player.SendChatMessage(COLOR_SUCC, "ָ�� GangZone ���Ƴ�");
}

DIALOG(DIALOG_TELEPORT_MAIN, DIALOG_STYLE_LIST, "����", "ȷ��", "ȡ��", false) {
	switch(listitem) {
	case 0: /* Create */
		DlgMgr.Show(DIALOG_TELEPORT_CREATE, "Ϊ���͵�����", player.GetId());
		break;

	case 1: /* Use */
		DlgMgr.Show(DIALOG_TELEPORT_USE, "���͵������", player.GetId());
		break;

	case 2: /* ViewNearby */ {
		CoordinatePlane center(player.GetPlaneCoordinate());
		mongo::BSONObj query(BSON("xy" << BSON("$near" << BSON_ARRAY(center.x << center.y))));
		std::auto_ptr<mongo::DBClientCursor> results(GetDB().query(CONFIG_STRING("Database.waypoint"), query, 16));
		std::stringstream str;
		while(results->more()) {
			mongo::BSONObj item(results->next());
			str << item["_id"].OID() << "\t" << item["title"].String() << "\n";
		}
		DlgMgr.Show(DIALOG_TELEPORT_NEARBY, str.str(), player.GetId());
		break;
	}

	case 3: /* CreateTrigger */
		DlgMgr.Show(DIALOG_TELEPORT_CREATETRIGGER, "���͵������", player.GetId());
		break;

	default:
		break;
	}
}

DIALOG(DIALOG_TELEPORT_CREATE, DIALOG_STYLE_INPUT, "�������͵�", "����", "ȡ��", false) {
	if(inputtext[0] == 0) return DlgMgr.Show(DIALOG_TELEPORT_CREATE, "���Ʋ���Ϊ��", player.GetId());
	Waypoint create(player.GetDetailedPos());
	create.Create(inputtext, player.GetUniqueID());
	player.SendChatMessage(COLOR_SUCC, "�Ѵ������͵� " + std::string(inputtext));
}

DIALOG(DIALOG_TELEPORT_USE, DIALOG_STYLE_INPUT, "���͵�", "����", "ȡ��", false) {
	if(inputtext[0] == 0) return DlgMgr.Show(DIALOG_TELEPORT_USE, "���Ʋ���Ϊ��", player.GetId());
	Waypoint point(inputtext);
	point.PerformTeleport(player.GetId());
}

DIALOG(DIALOG_TELEPORT_NEARBY, DIALOG_STYLE_LIST, "�鿴�����Ĵ��͵�", "����", "ȡ��", false) {
	Waypoint target(mongo::OID(std::string(inputtext, 24)));
	target.PerformTeleport(player.GetId());
}

DIALOG(DIALOG_TELEPORT_CREATETRIGGER, DIALOG_STYLE_INPUT, "�������ͱ��", "����", "ȡ��", false) {
	Waypoint wp(inputtext);
	CreateTeleportTrigger(wp.GetUniqueID(), player.GetPos());
	player.SendChatMessage(COLOR_SUCC, "�Ѵ����� " + std::string(inputtext) + " �Ĵ��ͱ��");
}

DIALOG(DIALOG_SERVER_MAIN, DIALOG_STYLE_LIST, "���������", "ȷ��", "ȡ��", false) {
	switch(listitem) {
	case 0: /* ReloadTeamList */
		TeamMgr.LoadAllFromDatabase();
		break;

	case 1: /* ReloadGangZoneList */
		GangZoneMgr.LoadAllFromDatabase();
		break;

	case 2: /* ReloadConfig */
		ReloadConfig();
		break;

	default:
		break;
	}
}

DIALOG(DIALOG_PLAYER_MAIN, DIALOG_STYLE_LIST, "���", "ȷ��", "ȡ��", false) {
	int target(player.GetVar<int>("player_lastclicked"));
	switch(listitem) {
	case 0: // SendPrivateMessage
		DlgMgr.Show(DIALOG_PLAYER_MESSAGE, "�� " + ProfileMgr[target].GetName()
			+ " ������Ϣ", player.GetId());
		break;

	default:
		break;
	}
}

DIALOG(DIALOG_PLAYER_MESSAGE, DIALOG_STYLE_INPUT, "������Ϣ", "����", "ȡ��", false) {
	ProfileMgr[player.GetVar<int>("player_lastclicked")].SendChatMessage(COLOR_SUCC, "���� " + player.GetName()
		+ " ����Ϣ: " + inputtext);
	player.SendChatMessage(COLOR_SUCC, "��Ϣ�ѷ���");
}

DIALOG(DIALOG_PROFILE_SETTING, DIALOG_STYLE_LIST, "��������", "ȷ��", "ȡ��", false) {
	switch(listitem) {
	case 0: // SetNickname
		DlgMgr.Show(DIALOG_PROFILE_SETTING_NICKNAME, "�������ǳ�", player.GetId());
		break;

	case 1: // Sync
		player.Sync();
		player.SendChatMessage(COLOR_SUCC, "������ͬ��");
		break;

	case 2: // ForceClassSelection
		player.ForceReselectPlayerClass();
		player.ToggleSpectating(true);
		player.ToggleSpectating(false);
		break;

	case 3: // Selfkill
		player.SetHealth(0.0f);
		break;

	default:
		break;
	}
}

DIALOG(DIALOG_PROFILE_SETTING_NICKNAME, DIALOG_STYLE_INPUT, "�����ǳ�", "ȷ��", "ȡ��", false) {
	if(inputtext[0] == 0) throw std::runtime_error("�ǳƲ���Ϊ��");
	player.SetNickname(inputtext);
	player.SendChatMessage(COLOR_SUCC, "�ǳƸ���Ϊ" + std::string(inputtext));
}
