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
	TS_STARTED,					//Задача работает
	TS_ENDED,					//Задача закончила работу
	TS_IO_FILE_BLOCKED,			//Задача ожидает окончание файловой операции
	TS_IO_USER_BLOCKED,			//Задача ожидает решения пользователя по какому-либо вопросу
	TS_IO_TASKRESPONSE_BLOCKED	//Задача ожидает ответа другой задачи
};


class User;
class Timetable;

/*
*	Абстрактный класс, описывающий некоторую задачу
*	Задача - некоторый алгоритм, который требуется выполнить отдельно от другого алгоритма
*	Предполагается, что задачи выполняются в отдельном потоке
*	Также, задача не возвращает результат, а выполняет работу над переданными данными.
*	Задаче можно передать некоторые параметры выполнения, или не передавать вовсе.
*
*	Для каждой отдельной задачи необходимо наследоваться от Task, или создавать объект SimpleTask или SimpleParametrizedTask, передавая туда
*	лямбду для работы класса.
*	Для взаимодействия между задачами существует класс CalendarApp и MessageQueue
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
*	Класс управления приложением
*
*	Этот класс управляет всеми выполняемыми задачами (Task),
*	потоками (через задачи), памятью (ограниченно) и является связующим звеном между различными задачами
*	
*	
*	Является статическим классом
*	Является синглтоном
*	ЭТОТ КЛАСС НЕ НАСЛЕДУЕТСЯ
*	
*		Алгоритм запуска приложения Initialize(argc, argv[]):
*			1.	Чтение входных параметров
*			2.	Выполнение Bootstrap() со входными параметрами
*			3.	Загрузка данных (выполнение LoadData())
*			4.	Инициализация классов работы (выполнение InitializeClasses())

*		Работа приложения обеспечивается выполнением Run()
*		
*		Алгоритм выключения ~CalendarApp():
*			1.	Создаётся задача Task - KillTask в отдельном потоке. Главный поток ожидает завершения Task
*				Посылается сигнал KillSignal от KillTask (KT) всем выполняемым работам.
*				Каждая работа может ответить на это сообщение, отправив ответ OK или отправив PromptUserMessage с объектом класса UserMessage
*				После того, как пройдет MaxResponseWaitTime или все задачи отправят ответ: OK, удаляются все задачи из очереди задач. Завершается KillTask
*			2.	После того, как основной поток дождался Task, выполняется Finalize()
*			3.	После этого выполняется SaveData()
*			4.	В конце выполняется остальной код ~CalendarApp()
*/	

class CalendarApp {
private:
	///////////////// ВСПОМОГАТЕЛЬНЫЕ КЛАССЫ /////////////
	//Задача удаления других задач (лол)
	class KillTask : Task {

		virtual void Run() override {

		};
	};
	////////////////////////////////////////////////////////


	static CalendarApp* ptr_; //@see "Singleton"


	//работающие задачи - все создаваемые задачи регистрируются здесь
	std::vector<std::shared_ptr<Task>> running_tasks_;
	
	//инициализация параметров класса
	CalendarApp() {

	}



#pragma region Class Init

	void Bootstrap() {
		//пока ничего
		
	}

	void LoadData() {
		//Загружаем файл юзера - user-settings.cu

	}

	void ClassInitialize() {

	}

	

#pragma endregion
public:


	//Возвращает объект приложения CalendarApp
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

