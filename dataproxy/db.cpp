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
