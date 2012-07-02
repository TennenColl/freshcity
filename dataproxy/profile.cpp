#include "common.h"
#define FCEXPORTIMPL
#include "profile.h"
#include "config.h"
#include "db.h"
#include "basemembsers.h"
#include "../common/exception.h"
#include "../common/logging.h"
#include "../common/hash.h"

void Profile::FlushPasswordHash() {
	try {
		_passwordhash = _members->rawdata.getObjectField("auth").getField("password").String();
	} catch(...) {
		LOGERROR("λ�� " + GetConfig().GetAttribute("Database.profile").ToString() + " ����Ч�û����� " + _uniqueid);
		throw FCException("������������û��ĵ�");
	}
}

Profile::Profile(const std::string& profileid)
	: BaseObject(GetConfig().GetAttribute("Database.profile").ToString(), profileid) {
		FlushPasswordHash();
}

bool Profile::CheckPassword(const std::string& input) const {
	return GetPasswordDigest(input) == _passwordhash;
}

void Profile::SetPassword(const std::string& newpassword) {
	DBInstance::GetDB().update(GetConfig().GetAttribute("Database.profile").ToString(), 
		BSON("_id" << mongo::OID(_uniqueid)), BSON("$set" << BSON("auth.password" << GetPasswordDigest(newpassword))));
	Synchronize();
}

void Profile::Synchronize() {
	BaseObject::Synchronize();
	FlushPasswordHash();
}
