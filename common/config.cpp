#define FCEXPORTIMPL
#include "../common/exception.h"
#include "../common/logging.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "config.h"

struct ConfigFile::Parser {
	boost::property_tree::ptree parser;
};

ConfigFile::ConfigFile(const std::string& filename) : _filename(filename), _parser(new Parser) {
	Reload();
}

AttributeElement ConfigFile::GetAttribute(const std::string& path) const {
	try {
		return AttributeElement(_parser->parser.get<std::string>(path));
	} catch(...) {
		LOGERROR("��Ч·�� " + path);
		throw FCException("ָ��·����Ч");
	}
}

void ConfigFile::SetAttribute(const std::string& path, const std::string& value) {
	try {
		_parser->parser.get(path, value);
	} catch(...) {
		LOGERROR("��Ч·�� " + path);
		throw FCException("ָ��·����Ч");
	}
}

void ConfigFile::Reload() {
	try {
		boost::property_tree::ini_parser::read_ini(_filename, _parser.get()->parser);
	} catch(...) {
		LOGERROR("�޷����ļ� " + _filename);
		throw FCException("�޷���ָ���ļ�");
	}
}

void ConfigFile::Save() {
	try {
		boost::property_tree::ini_parser::write_ini(_filename, _parser.get()->parser);
	} catch(...) {
		LOGERROR("������д�뵽�ļ� " + _filename + " ʱ��������");
		throw FCException("�޷�����ָ���ļ�");
	}
}
