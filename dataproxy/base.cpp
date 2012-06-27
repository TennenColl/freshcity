#include "common.h"
#define FCEXPORTIMPL
#include <boost/unordered_map.hpp>
#include "../common/exception.h"
#include "../common/logging.h"
#include "db.h"
#include "base.h"
#include "config.h"

struct BaseObject::AttributeMap {
	boost::unordered_map<std::string, boost::shared_ptr<AttributeElement>> map;
};

BaseObject::BaseObject(const std::string& uniqueid)	: _uniqueid(uniqueid),
	_collection(dbconfig.GetAttribute("Database.undefined").ToString()), _attributes(new BaseObject::AttributeMap) {
		ConnectDatabase();
}

void BaseObject::SetAttribute(const std::string& key, const AttributeElement& value) {
	_attributes->map[key] = boost::shared_ptr<AttributeElement>(new AttributeElement(value));
}

void BaseObject::RemoveAttribute(const std::string& key) {
	try {
		_attributes->map.erase(key);
	} catch(...) {
		LOGERROR("�����ڵļ�ֵ " + key);
		throw FCException("����ɾ��һ�Բ����ڵļ�ֵ");
	}
}

AttributeElement BaseObject::GetAttribute(const std::string& key) {
	try {
		return *_attributes->map.at(key).get();
	} catch(...) {
		LOGERROR("�����ڵļ�ֵ " + key);
		throw FCException("���Բ��Ҳ���ȡ�����ڵ�ֵ");
	}
}

void BaseObject::Submit() {
	mongo::BSONObjBuilder attributelist;
	boost::unordered_map<std::string, boost::shared_ptr<AttributeElement>>& map = _attributes->map;
	boost::unordered_map<std::string, boost::shared_ptr<AttributeElement>>::iterator mapiter = map.begin();

	for(mapiter; mapiter != map.end(); mapiter++) {
		switch(mapiter->second->GetValueType()) {
		case AttributeElement::Type::STRING:
			attributelist.append("attribute." + mapiter->first, mapiter->second->ToString());
			break;

		case AttributeElement::Type::NUMBER:
			attributelist.append("attribute." + mapiter->first, mapiter->second->ToNumber());
			break;

		case AttributeElement::Type::BOOL:
			attributelist.append("attribute." + mapiter->first, mapiter->second->ToBool());
			break;
			
		default:
			break;
		}
	}

	try {
		dbconnection.update(_collection, BSON("_id" << mongo::OID(_uniqueid)), BSON("$set" << attributelist.obj()), true);
	} catch(mongo::UserException& e) {
		LOGERROR("�ύ��������ʱ��������: " + e.toString());
		throw FCException("�޷������������");
	}

	return;
}
