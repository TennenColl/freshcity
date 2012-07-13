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
#include "application_data_waypoint.h"
#include "application_config.h"
#include <sampgdk/a_players.h>
#include <sampgdk/a_vehicles.h>

void Waypoint::_LoadData() {
	if(_existsindatabase) {
		try {
			std::vector<mongo::BSONElement> coordinate = _rawdata["xy"].Array();
			_waypoint = Coordinate5D((float)coordinate[0].Number(),
				(float)coordinate[1].Number(), (float)_rawdata["z"].Number(),
				(int)_rawdata["world"].Number(), (int)_rawdata["interior"].Number());
		} catch(mongo::UserException) {
			throw std::runtime_error("��Ч���͵��ĵ�");
		}
	} else {
		throw std::runtime_error("�����ڵĴ��͵��ĵ�");
	}
}

Waypoint::Waypoint(const std::string& title)
	: SingleObject(CONFIG_STRING("Database.waypoint"), BSON("title" << title)) {
		_LoadData();
}

Waypoint::Waypoint(const mongo::OID& id)
	: SingleObject(CONFIG_STRING("Database.waypoint"), id) {
		_LoadData();
}
Coordinate5D Waypoint::Get() const {
	return _waypoint;
}

void Waypoint::ApplyToPlayer(int playerid) const {
	SetPlayerVirtualWorld(playerid, _waypoint.virtualworld);
	SetPlayerInterior(playerid, _waypoint.interior);
	SetPlayerPos(playerid, _waypoint.x, _waypoint.y, _waypoint.z);
}

void Waypoint::ApplyToVehicle(int vid) const {
	SetVehicleVirtualWorld(vid, _waypoint.virtualworld);
	LinkVehicleToInterior(vid, _waypoint.interior);
	SetVehiclePos(vid, _waypoint.x, _waypoint.y, _waypoint.z);
}

mongo::OID CreateWaypoint(const std::string& title, const Coordinate5D& point, const mongo::OID& creator) {
	mongo::BSONObj submit = BSON(mongo::GENOID <<
		"title"		<< title <<
		"creator"	<< creator <<
		"xy"		<< BSON_ARRAY(point.x << point.y) <<
		"z"			<< point.z <<
		"world"		<< point.virtualworld <<
		"interior"	<< point.interior);
	SingleObject submitter(CONFIG_STRING("Database.waypoint"));
	submitter.Create(submit);
	return submitter.GetID();
}