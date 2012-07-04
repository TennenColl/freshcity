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

#include "common.h"
#include "../common/exception.h"
#include "../common/logging.h"
#include "../common/config.h"
#include "db.h"
#include "config.h"
#include "../mongo/client/connpool.h"

mongo::DBClientConnection& DBInstance::GetDB() {
	static mongo::DBClientConnection dbconnection;
	static bool _connected;

	if(!_connected) {
		std::string host(GetConfig().GetAttribute("Database.host").ToString());
		LOGINFO("�������ӵ� " + host + " �� mongodb ������");
		try {
			dbconnection.connect(host);
			_connected = true;
			LOGINFO("������");
		} catch(mongo::DBException &e) {
			LOGERROR("�������ݿ�ʱ��������: " + e.toString());
			throw FCException("�޷����������ݿ�");
		}
	}

	return dbconnection;
}
