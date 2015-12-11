#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <mutex>

#include "../CommonLib/json/json.h"
#include "../CommonLib/CommandLine.h"
#include "../CommonLib/MessageQueue.hpp"

class CalendarApp;



enum TaskStatus {
	TS_STARTED,					//������ ��������
	TS_ENDED,					//������ ��������� ������
	TS_IO_FILE_BLOCKED,			//������ ������� ��������� �������� ��������
	TS_IO_USER_BLOCKED,			//������ ������� ������� ������������ �� ������-���� �������
	TS_IO_TASKRESPONSE_BLOCKED	//������ ������� ������ ������ ������
};


class User;
class Timetable;

/*
*	����������� �����, ����������� ��������� ������
*	������ - ��������� ��������, ������� ��������� ��������� �������� �� ������� ���������
*	��������������, ��� ������ ����������� � ��������� ������
*	�����, ������ �� ���������� ���������, � ��������� ������ ��� ����������� �������.
*	������ ����� �������� ��������� ��������� ����������, ��� �� ���������� �����.
*
*	��� ������ ��������� ������ ���������� ������������� �� Task, ��� ��������� ������ SimpleTask ��� SimpleParametrizedTask, ��������� ����
*	������ ��� ������ ������.
*	��� �������������� ����� �������� ���������� ����� CalendarApp � MessageQueue
*/
class Task:std::enable_shared_from_this<Task> {
private:
	std::shared_ptr<std::thread> thread_;
	std::mutex operation_mutex_;

	TaskStatus status_;
protected:
	
public:
	Task() {
		CalendarApp::Current().RegisterTask(this);
	}


	bool Wait();
	bool Wait(int milliseconds);


	virtual void Run() = 0;


};



class ConsoleUserInputTask : Task {

	void Run() override {

	}
};

/*
*	����� ���������� �����������
*
*	���� ����� ��������� ����� ������������ �������� (Task),
*	�������� (����� ������), ������� (�����������) � �������� ��������� ������ ����� ���������� ��������
*	
*	
*	�������� ����������� �������
*	�������� ����������
*	���� ����� �� �����������
*	
*		�������� ������� ���������� Initialize(argc, argv[]):
*			1.	������ ������� ����������
*			2.	���������� Bootstrap() �� �������� �����������
*			3.	�������� ������ (���������� LoadData())
*			4.	������������� ������� ������ (���������� InitializeClasses())

*		������ ���������� �������������� ����������� Run()
*		
*		�������� ���������� ~CalendarApp():
*			1.	�������� ������ Task - KillTask � ��������� ������. ������� ����� ������� ���������� Task
*				���������� ������ KillSignal �� KillTask (KT) ���� ����������� �������.
*				������ ������ ����� �������� �� ��� ���������, �������� ����� OK ��� �������� PromptUserMessage � �������� ������ UserMessage
*				����� ����, ��� ������� MaxResponseWaitTime ��� ��� ������ �������� �����: OK, ��������� ��� ������ �� ������� �����. ����������� KillTask
*			2.	����� ����, ��� �������� ����� �������� Task, ����������� Finalize()
*			3.	����� ����� ����������� SaveData()
*			4.	� ����� ����������� ��������� ��� ~CalendarApp()
*/	

class CalendarApp {
private:
	///////////////// ��������������� ������ /////////////
	//������ �������� ������ ����� (���)
	class KillTask : Task {

		virtual void Run() override {

		};
	};
	////////////////////////////////////////////////////////


	static CalendarApp* ptr_; //@see "Singleton"


	//���������� ������ - ��� ����������� ������ �������������� �����
	std::vector<std::shared_ptr<Task>> running_tasks_;
	
	//������������� ���������� ������
	CalendarApp() {

	}



#pragma region Class Init

	void Bootstrap() {
		//���� ������
		
	}

	void LoadData() {
		//��������� ���� ����� - user-settings.cu

	}

	void ClassInitialize() {

	}

	

#pragma endregion
public:


	//���������� ������ ���������� CalendarApp
	static CalendarApp& Current() {
		if (!CalendarApp::ptr_)
			CalendarApp::ptr_ = new CalendarApp();
	}


#pragma region Workflow
	void Initialize(int argc, char** argv) {

	}


	void Run() {
		std::shared_ptr<Task> sp(new ConsoleUserInputTask());
		running_tasks_.push_back(sp);
		sp->Run();
	}

	void SwitchUser(const User& user) {
		throw std::exception("Not implemented");

	}

	void Shutdown() {
		throw std::exception("Not implemented");
	}
	
#pragma endregion

#pragma region Registrar Methods
	void RegisterTask(const std::shared_ptr<Task> &t) {
		this->running_tasks_.push_back(t);
	}


#pragma endregion

	~CalendarApp() {
		KillTask kt;
	}
};

