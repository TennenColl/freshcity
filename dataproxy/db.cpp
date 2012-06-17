#include "common.h"
#include "../common/exception.h"
#include "../common/logging.h"
#include "../common/config.h"
#include "db.h"

mongo::DBClientConnection connection_(true);
ConfigFile dbconfig("freshcity.ini");

class DBInit {
public:
	DBInit() {
		std::string host(dbconfig.GetAttribute("Database.host").ToString());
		LOGINFO("�������ӵ� " + host + " �� mongodb ������");
		try {
			connection_.connect(mongo::HostAndPort(host));
			LOGINFO("������");
		} catch(mongo::DBException &e) {
			throw FCException("�������ݿ�ʱ��������: " + e.toString());
		}
	}
};

DBInit dbinitinstance;

mongo::DBClientConnection &GetDB() {
	return connection_;
}
