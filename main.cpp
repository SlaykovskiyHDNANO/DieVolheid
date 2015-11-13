#include <QCoreApplication>
#include <QVector>
#include <QMultiMap>
#include <QSet>
#include "base.h"

void test1();
void test2();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    test2();
    return a.exec();
}


void test2()
{
    QMultiMap<QString, quint8> qm;
    qm.insert("123", 5);
    qm.insert("234", 6);
    qm.insert("234", 5);
    qm.insert("123", 5);
    qm.insert("123", 5);
    QMultiMap<QString, quint8>::iterator it = qm.find("123");
    qm.erase(it);
    QVector<quint8> v = qm.values().toSet().toList().toVector();

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
}

void test1()
{
    Task t("123", QVector<Layer *>(0), QDateTime(QDate(1,1,1), QTime(1,1,1)), QDateTime(QDate(2,2,2), QTime(2,2,2)), "MyTitle", "MyDescr", false);
    QJsonObject ob;
    t.Serialize(ob);
    Task t2;
    t2.Deserialize(ob);
}
