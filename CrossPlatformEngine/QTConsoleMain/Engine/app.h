#ifndef APP_H
#define APP_H

#include <QObject>
#include <memory>
#include "base.h"

class App : public QObject
{
    Q_OBJECT
protected:
    static std::shared_ptr<App> current_;
private:
    explicit App();
public:
    static App& Current();

    virtual ~App();

signals:

public slots:
};


class CalendarApp: protected App
{
    Q_OBJECT
public:

    static CalendarApp& Current();

};

#endif // APP_H
