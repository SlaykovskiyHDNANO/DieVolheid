#include <QCoreApplication>
#include "base.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Layer* l1 = new Layer("12345" , QVector<TimeSpan*>(), QVector<Layer*>(0), 0, "root", "im a root");
    Layer* l2 = new Layer("123456" , QVector<TimeSpan*>(), QVector<Layer*>(0), 0, "son", "im a son");
    l1->AddSublayer(l2);
    TimeSpan* x = new TimeSpan("1", QVector<Layer *>(0), QDateTime(QDate(1,1,1), QTime(1,1,1)), QDateTime(QDate(2,2,2), QTime(2,2,2)), "MyTitle1", "MyDescr1");
    TimeSpan* y = new TimeSpan("2", QVector<Layer *>(0), QDateTime(QDate(4,4,4), QTime(4,4,4)), QDateTime(QDate(5,5,5), QTime(5,5,5)), "MyTitle2", "MyDescr2");
    l1->AddInterval(x);
    l1->AddInterval(y);
    l2->AddInterval(x);
    l2->AddInterval(y);
    QJsonObject ob;
    l1->Serialize(ob, 2, Layer::HEAVY_SERIALIZATION);
    Layer* l = new Layer();
    l->Deserialize(ob, 2, Layer::HEAVY_DESERIALIZATION);
    return a.exec();
}

void test1()
{
    Task t("123", QVector<Layer *>(0), QDateTime(QDate(1,1,1), QTime(1,1,1)), QDateTime(QDate(2,2,2), QTime(2,2,2)), "MyTitle", "MyDescr", false);
    QJsonObject ob;
    t.Serialize(ob);
    Task t2;
    t2.Deserialize(ob);
}
