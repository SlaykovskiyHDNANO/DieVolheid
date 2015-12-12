#include "app.h"

std::shared_ptr<App> App::current_ = nullptr;

App::App() : QObject(0)
{

}

App& App::Current()
{
    if(!App::current_)
        App::current_ = std::shared_ptr<App>(new App());
    return *App::current_;
}

App::~App()
{

}

