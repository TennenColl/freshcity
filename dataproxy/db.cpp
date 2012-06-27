#include "common.h"
#include "../common/exception.h"
#include "../common/logging.h"
#include "../common/config.h"
#include "db.h"
#include "config.h"

mongo::DBClientConnection dbconnection(true);
bool connected = false;

void ConnectDatabase() {
	if(connected) return;
	std::string host(dbconfig.GetAttribute("Database.host").ToString());
	LOGINFO("�������ӵ� " + host + " �� mongodb ������");
	try {
		dbconnection.connect(host);
		LOGINFO("������");
	} catch(mongo::DBException &e) {
		LOGERROR("�������ݿ�ʱ��������: " + e.toString());
		throw FCException("�޷����������ݿ�");
	}
}
