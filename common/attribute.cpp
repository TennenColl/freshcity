#define FCEXPORTIMPL
#include "attribute.h"
#include "exception.h"
#include <boost/lexical_cast.hpp>

AttributeElement::AttributeElement(const std::string& value) : _value(value) {}

std::string AttributeElement::ToString() {
	return _value;
}

double AttributeElement::ToNumber() {
	try {
		return boost::lexical_cast<double>(_value);
	} catch (...) {
		throw FCException("�޷���ֵ " + _value + " ת��Ϊ double");
	}
}

bool AttributeElement::ToBool() {
	try {
		return boost::lexical_cast<bool>(_value);
	} catch (...) {
		throw FCException("�޷���ֵ " + _value + " ת��Ϊ bool");
	}
}
