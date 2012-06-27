#define FCEXPORTIMPL
#include "attribute.h"
#include "exception.h"
#include <boost/lexical_cast.hpp>
#include "logging.h"

AttributeElement::AttributeElement(const std::string& value) : _type(Type::STRING), _value(value) {}

AttributeElement::AttributeElement(double value) : _type(Type::NUMBER), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::AttributeElement(bool value) : _type(Type::BOOL), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::Type AttributeElement::GetValueType() { 
	return _type;
}

std::string AttributeElement::ToString() {
	return _value;
}

double AttributeElement::ToNumber() {
	try {
		return boost::lexical_cast<double>(_value);
	} catch (...) {
		LOGERROR("�޷���ֵ " + ToString() + " ת��Ϊ double");
		throw FCException("����ת��ʧ��");
	}
}

bool AttributeElement::ToBool() {
	try {
		return boost::lexical_cast<bool>(_value);
	} catch (...) {
		LOGERROR("�޷���ֵ " + ToString() + " ת��Ϊ bool");
		throw FCException("����ת��ʧ��");
	}
}
