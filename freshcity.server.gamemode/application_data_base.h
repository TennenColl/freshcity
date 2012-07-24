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

#ifndef FRESHCITY_APPLICATION_DATA_BASE
#define FRESHCITY_APPLICATION_DATA_BASE

#include "application_database.h"

/* �������ݿ����Ļ��� �׳��쳣��Ϊ std::runtime_error */
class SaveableItem {
protected:
	/* �������ڵļ��� ��: freshcity.profile ����ͨ��Config��ȡ */
	std::string _collection;
	/* ����ԭʼ���� ���� SetData() ���� */
	mongo::BSONObj _rawdata;
	mongo::OID _uniqueid;

public:
	/* ����ͨ��ID��ȡ�Ĺ��캯�� */
	SaveableItem(const std::string& collection, const mongo::OID& uniqueid);
	/* ��������������ѯ�Ĺ��캯�� */
	SaveableItem(const std::string& collection, const mongo::BSONObj& query);
	/* ���ڴ����¶���(�������Ժ󱣴�)�Ĺ��캯�� */
	SaveableItem(const std::string& collection);
	/* ���»�ȡ�������� �� IsExistInDatabase() == false ���׳��쳣 */
	void Refetch();
	/* ���ö������� ����Ϊ�ն���ʱ�ſ��� */
	void InitData(const mongo::BSONObj& data);
	/* ���������ݱ��������ݿ� ���Ѵ���/����Ϊ��/����ʧ�����׳��쳣 ��ѡ�Ƿ����»�ȡ */
	void Create(const mongo::BSONObj& data, bool refetch);
	/* ���¶������� ��ѡ�Ƿ����»�ȡ */
	void Update(const mongo::BSONObj& modifier, bool refetch);
	mongo::OID GetUniqueID() const;
	bool IsEmpty() const;
};

#endif
