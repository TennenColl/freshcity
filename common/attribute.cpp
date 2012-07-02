#define FCEXPORTIMPL
#include "attribute.h"
#include "exception.h"
#include "logging.h"
#include <boost/lexical_cast.hpp>

AttributeElement::AttributeElement(const std::string& value) : _type(Type::STRING), _value(value) {}

AttributeElement::AttributeElement(const char* value) : _type(Type::STRING), _value(value) {}

AttributeElement::AttributeElement(double value) : _type(Type::NUMBER), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::AttributeElement(int value) : _type(Type::NUMBER), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::AttributeElement(long value) : _type(Type::NUMBER), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::AttributeElement(bool value) : _type(Type::BOOL), _value(boost::lexical_cast<std::string>(value)) {}

AttributeElement::Type AttributeElement::GetValueType() const { 
	return _type;
}

std::string AttributeElement::ToString() const {
	return _value;
}

double AttributeElement::ToNumber() const {
	try {
		return boost::lexical_cast<double>(_value);
	} catch (...) {
		LOGERROR("�޷���ֵ " + ToString() + " ת��Ϊ double");
		throw FCException("����ת��ʧ��");
	}
}

bool AttributeElement::ToBool() const {
	try {
		return boost::lexical_cast<bool>(_value);
	} catch (...) {
		LOGERROR("�޷���ֵ " + ToString() + " ת��Ϊ bool");
		throw FCException("����ת��ʧ��");
	}
}
