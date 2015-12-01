#pragma once
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <mongo/client/dbclient.h> // for the driver
#include <string>
#include <thread>
#include <mutex>
//SUUID - 4,8,12,16 �������� �����
//��� ��������� � ����� �� ��������� 16 ������� ������ (�������), ����� ������ � �����
//��� ��������� (���� ���) 32 ������� ������.
#include "suuid.hpp"

using namespace std;



enum SolvexDBObjectType {
	SOT_TAG,
	SOT_POST,
	SOT_USER,
	SOT_SECTION,
	SOT_PREPROCESSED_TEXT //�������� ��������
};

class IDBConnectorResult {
public:
	virtual mongo::BSONObj	NextResult(bool own)	const	= 0;
	virtual bool			HasMore()				const	= 0;
	virtual bool			HasError()				const	= 0;
	virtual std::string		Raw()					const	= 0;
};

class MongoQueryResult :IDBConnectorResult {
protected:
	auto_ptr<mongo::DBClientCursor> _q;
public:
	mongo::BSONObj	NextResult(bool own)	const	override;
	bool			HasMore()				const	override;
	bool			HasError()				const	override;
	std::string		Raw()					const	override;
};


struct DBConnectorStatus {
	void* data;
	bool connected;
	bool initialized;
	bool error;
	string errmsg;
	std::shared_ptr<mongo::DBException> excep;

	DBConnectorStatus();
};
struct DBConnectorCredentials {
	string dbuser;
	string dbname;
	string dbpass;
};

class DBConnectorBase:public enable_shared_from_this<DBConnectorBase>
{
protected:
	DBConnectorStatus status;
	std::shared_ptr<mongo::DBClientConnection> cn;
	std::shared_ptr<DBConnectorCredentials> credentials;
	shared_ptr<std::mutex> _m;
public:
#pragma region Constructors and Destructors
	//������������ � ���� ������, ����� ���� �������� ��� �������
	DBConnectorBase();
	DBConnectorBase(const DBConnectorBase&);
	DBConnectorBase(const string &dbname, const string &dbuser, const string &pwd, bool connect_now = true);
	~DBConnectorBase();
#pragma endregion
#pragma region Main
	bool Connect();
	bool Disconnect();
	bool Reconnect();
	//������ �������� dbname dbuser pwd �� ��������
	void SetCredentials(const string &dbname = "",const string &dbuser = "",const string &pwd = "",const bool &reconnect_now = false);
#pragma endregion
#pragma region Misc
	DBConnectorStatus Status() const;
	bool IsConnected() const;
	string LastErrorMsg() const;
	mongo::DBException LastError() const;
#pragma endregion


};

class DBConnectorWriter :public DBConnectorBase
{
public:
#pragma region Constructors and Destructors
	//������������ � ���� ������, ����� ���� �������� ��� �������
	DBConnectorWriter(){};
	DBConnectorWriter(const string &dbname, const string &dbuser, const string &pwd, const bool &connect_now = true);
	~DBConnectorWriter();
#pragma endregion

#pragma region Write-oriented methods
	bool Write(SolvexDBObjectType type, const mongo::BSONObj);
#pragma endregion

};

class DBConnectorReader:public DBConnectorBase
{
	static int seed;

	int sowed_seed;
public:
#pragma region Constructors and Destructors
	DBConnectorReader();
	DBConnectorReader(const string &dbname ,const string &dbuser, const string &pwd, const bool &connect_now = true);
	~DBConnectorReader();
#pragma endregion

#pragma region Read-oriented methods
	mongo::BSONObj GetById(SolvexDBObjectType type,const SUUID &id,const std::vector<string> &fields = {}, bool* ok = nullptr);
	//mongo::BSONObjSet GetByQuery(SolvexDBObjectType type,const string &query_string,const std::vector<string> &fields = {}, bool* ok = nullptr);
#pragma endregion

	friend bool DBConnectorReader::operator<(const DBConnectorReader& a, const DBConnectorReader& b) {
		return a.sowed_seed < b.sowed_seed;
	}

};

struct DBWriteEntry
{
	SolvexDBObjectType type;
	mongo::BSONObj obj;
};


class DBConnectorManager;

/** 
* ������������� ������ � ���� ������ ���� ������������. �������� ����� �������� �������� � �������� ����� ����������.
* ������ ��������� 
* ���� ����� �� ������ �������� ���� ������������
* �� ����� ������ � DBConnectorManager, ������� �������� ��������� �� ����������
* ����� ������� ������ ��� ������ ������ ��������� ������� StartAccess()
* ��� ������ ����� ��������� ������ ��������� ������� EndAccess(). ���, ��� ���� ������ �� ������, ��������� � mongoDB
**/
class DBConnectorAccessor:protected DBConnectorReader,public enable_shared_from_this<DBConnectorAccessor>
{
	DBConnectorManager* daddy;
	std::deque<DBWriteEntry> write_entries;
	int thread_id;
	bool access_granted;
	bool valid;
	bool end_access_called;
public:
	DBConnectorAccessor(const DBConnectorReader& reader, DBConnectorManager* daddy);
	DBConnectorAccessor(const DBConnectorAccessor&);
	DBConnectorAccessor();
	//��������� ������������ ����� ������. ���������� id ������
	bool StartAccess();
	//����� ����� ��������� ������������ ����� ������. �������� DBConnectorManager::ChildIsDone() 
	bool EndAccess();
	//DBConnectorAccessor& operator=(const DBConnectorAccessor& a);
	//����� ����� ��������� ����� ����������� ����, �������� �� ����� StartAccess()
	mongo::BSONObj GetById(SolvexDBObjectType type, const SUUID &id, const std::vector<string> &fields = {}, bool* ok = nullptr);
	//mongo::BSONObjSet GetByQuery(SolvexDBObjectType type, const string &query_string, const std::vector<string> &fields = {}, bool* ok = nullptr);
	
	bool EnqueueWrite(SolvexDBObjectType type, const mongo::BSONObj &object);
	//bool EnqueueWrite(SolvexDBObjectType type, const mongo::BSONObjSet &&objects);

	//��� ��������, ��� ������ DBConnectorManager �������� ��� ���� �� ����� ������. ���� ���������
	friend class DBConnectorManager;
};

class DBConnectorManager
{
	//��� ��� ��� ����� �������� ����� �������, �� �� ����� ��������� ������ � ������� ������ ������ ������,
	//��������� ����� ����� ��������� ������ �� ���� ������.
	std::mutex locker;
	bool	connected;
	//�������, � ������� �������� � ���-���������� � ����� � �������� (���������� ������ DBConnectorAccessor �� ��� ����������)
		
	DBConnectorWriter writer;
protected:
	//��� ������� ����� �������� ��� DBConnectorAccessor ����� ������ EndAccess()
	//DBConnectorManager ������ �� ������ ��� ������, ������� ������������ ���������� ����� �� ������.
	bool ChildIsDone(const DBConnectorAccessor& child);
public:
	std::map<DBConnectorReader, std::vector<DBConnectorAccessor*>> load;
	DBConnectorManager(const string &dbname, const string &dbuser, const string &pwd, int number_of_connections = 2, const bool& connect_now = false);

	//connect
	bool Connect(bool reconnect_connected = false);

	//Checks, whether connection to DB is made
	//@arg		writer Place to store a status of writer connection
	//@arg		any_reader Place to store condition result, whether any of reader is connected
	//@return	bool value, indicating whether ALL connections (readers and writers are connected)
	bool IsConnected(bool* writer = nullptr, bool* any_reader = nullptr);

	//���������� ����� DBAccessor.
	//�������� DBConnectorReader � ���������� ����������� DBConnectorAccessor � ������� DBConnectorAccessor � ��������� ����� ���������� ������������ DBConnectorReader
	DBConnectorAccessor& GetAccessor();
	
	//������� "�����������": ��� ��������� �������� ������ �����������, ����� �� ������ ������� EndAccess(), ���?
	//�� ��� ���, ����� ��� �� ����� DBConnectorAccessor, �� ���������� id ������� ������ � �������� ����.
	//manager ������ ��� DBConnectorAccessor, � ������� ���� id � ������ ��
	bool RemoveAccessorsByThreadId(int thread_id);

	friend class DBConnectorAccessor;
};


