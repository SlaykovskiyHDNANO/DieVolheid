#include <QCoreApplication>
#include "Console/CommandLine.h"
#include "Engine/base.h"
#include <QString>
#include <iostream>

class CalendarMainWorkflow: public QCoreApplication {
    CommandLine cmd = {
        CommandLine({
           Command("create_user",0, { Option("n","name",nullptr, "Sets username for user",true,false)}, "This command creates user."),
           Command("create_layer",1, {}, "This command creates layer for user"),
           Command("save",2, { Option("f","file",nullptr,"Filename to save",true)}, "This command saves current state into file"),
           Command("create_event",3,{}, "This command creates event for user in layer(s)")

        }, -1, "This is the calendar!")
    };

    int argc;
    char** argv;
public:
    CalendarMainWorkflow(int argc, char *argv[]):QCoreApplication(argc, argv),argc(argc),argv(argv) {

    }

    int exec() {
        cmd.SelfTest();
        QStringList list = this->arguments();
        cmd.Parse(argc,argv);
        std::cout << cmd.Help() << std::endl;
        return 0;
    }
};


int main(int argc, char *argv[])
{
    CalendarMainWorkflow a(argc, argv);

    return a.exec();
}

