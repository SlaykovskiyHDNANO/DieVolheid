#ifndef BASE_H
#define BASE_H
#include <QVector>
#include <QString>
#include <QDate>
#include <QFile>
class TimeSpan;
class Layer//слой
{
    long long           id;//В качестве id пока используется long long, затем он будет заменён
    QVector <TimeSpan*> intervals;//Вектор содержащие отрезки времени(события или задачи)
    Layer*              ParentLayer;//Родительский слой, то есть такой слой, для которого данный слой является подслоем
    QVector <Layer*>    sublayers;//подслои
    QString             title;//заголовок
    QString             description;//описание
public:
    Layer ();
    Layer (long long Id, QVector<TimeSpan*> Intervals,QVector <Layer*> Sublayers,
           QString Title, QString Description);

    long long          GetID();//Получить id
    QString            GetTitle();// Получить заголовок
    QString            GetDescription();//Получить описание
    Layer*             GetParrentLayer();//Получить родительский слой
    QVector<Layer*>    GetSublayers(int MaxDepth);//Получение всех подслоёв глубины не более MaxDepth
    QVector<TimeSpan*> GetAllIntervals();//Получение всех интервалов в слое, включая интервалы подслоёв,
    // интервалы подслоёв подслоёв и т.д. Причём полученный вектор содержит интервалы в порядке возрастания
    // даты начала и не содержит одинаковых интервалов

    void SetTitle       (QString NewTitle);// Сделать значением заголовка NewTitle
    void SetDescription (QString NewDescription);//  Сделать значением описания NewDescription
    void SetParentLayer (Layer* NewParent);//  Сделать родительским слоем NewParent
    void SetParentLayer (long long Id);// Сделать родительским слоем, слой с id равным Id

    void AddInterval  (long long Id);// Добавить в слой существующий интервал с id равным Id
    void AddInterval  (TimeSpan* Interval);// Добавить в слой  существующийинтервал Interval
    void NewInterval  (QDate Start, QDate End, QString IntervalTitle, QString IntervalDescription);
    //Создать новый интервал с датой начала Start, датой окончания End,
    //заголовком IntervalTitle,  описанием IntervalDescription.
    void AddIntervals (QVector<long long> IntervalsId);// Добавить интервалы с id IntervalsId
    void AddIntervals (QVector<TimeSpan*> IntervalsPtr);// Добавить интеревалы, адресуемые указателями IntervalsPtr

    void AddSublayer  (long long Id);// Сделать подслоем слой с id равным Id
    void AddSublayer  (Layer* Sublayer);//Сделать подслоем слой, адресуемый укзателем Sublayer
    void NewSublayer  (QString SublayerTitle, QString SublayerDescription);
    //Создать новый подслой заголовком SublayerTitle,  описанием SublayerDescription.
    void AddSublayers (QVector<long long> SublayersId);//Добавить Добавить подслои с id SublayersId
    void AddSublayers (QVector<Layer*> SublayersPtr);// Добавить подслои, адресуемые указателями SublayersPtr

    void DeleteInterval  (long long Id);// Удалить из слоя интервал с id равным Id
    void DeleteInterval  (TimeSpan* Interval);// Удалить из слоя интервал, адресуемый укзателем Interval
    void DeleteIntervals (QVector<long long> IntervalsId);// Удалить из слоя интервалы с id IntervalsId
    void DeleteIntervals (QVector<TimeSpan*> IntervalsPtr);// Удалить из слоя интеревалы, адресуемые указателями IntervalsPtr

    void DeleteSublayer  (long long Id);// Удалить подслой с id равным Id
    void DeleteSublayer  (Layer* Sublayer);// Удалить подслой, адресуемый укзателем Sublayer
    void DeleteSublayers (QVector<long long> SublayersId);//Удалить подслои с id SublayersId
    void DeleteSublayers (QVector<Layer*> SublayersPtr);// Удалить подслои, адресуемые указателями SublayersPtr

    void Clear();//Очитка слоя, т.е. удаление из него всех подслоёв,интервалов, заголовка, описания.
    void Serialize(QFile File);//Сериализация
    void Deserialize(QFile File);//Десериализация
};

class TimeSpan//Временной промежуток
{
    long long        id;//В качестве id пока используется long long, затем он будет заменён
    QVector <Layer*> layers;//слои, связанные с промежутком времени
    QDate            start;//Начала интервала
    QDate            end;//Конец интервала
    QString          title;//заголовок
    QString          description;//описание
public:
    TimeSpan();
    TimeSpan (long long Id, QVector<Layer*> Layers,QDate Start,
           QDate End,QString Title, QString Description);

    long long        GetID();//Получить id
    QString          GetTitle();// Получить заголовок
    QString          GetDescription();//Получить описание
    QDate            GetStart();//Получить начало отрезка
    QDate            GetEnd();//Получить конец отрезка
    QVector <Layer*> GetLayers();// Получить, связанные с промежутком слои


    void SetTitle(QString NewTitle);//  Сделать значением заголовка NewTitle
    void SetDescription(QString NewDescription);//  Сделать значением описания NewDescription
    void SetStart(QDate NewStart);//  Сделать значением начала отрезка NewStart
    void SetEnd(QDate NewEnd);    //  Сделать значением конца отрезка NewEnd

    void AddLayer  (long long Id);//Добавить слой c id, равныи Id.
    void AddLayer  (Layer* NewLayer);//Добавить слой NewLayer
    void AddLayers (QVector<long long> LayersId);//Добавить слои c id LayersId
    void AddLayers (QVector<Layer*> LayersPtr);//Добавить слои, адресуемые указателями LayersPtr

    void DeleteLayer  (long long Id);//Удалить слой c id, равныи Id.
    void DeleteLayer  (Layer* DeletedLayer);//Удалить слой NewLayer
    void DeleteLayers (QVector<long long> LayersId);//Удалить слои c id LayersId
    void DeleteLayers (QVector<Layer*> LayersPtr);//Удалить слои, адресуемые указателями LayersPtr

    virtual void Serialize  (QFile File);//Сериализация
    virtual void Deserialize(QFile File);//Десериализация
};
class Task: public TimeSpan//Задача
{
    bool is_performed;// выполнена?
public:
    Task();
    Task (long long Id, QVector<Layer*> Layers,QDate Start,
           QDate End,QString Title, QString Description, bool IsPerformed);

    bool GetIsPerformed();
    void SetIsPerformed(bool IsPerformed);

    virtual void Serialize  (QFile File);//Сериализация
    virtual void Deserialize(QFile File);//Десериализация

};


class Event: public TimeSpan//Событие
{
    QVector<Task> tasks;//Задачи, входящие в событие
public:
    Event();
    Event (long long Id, QVector<Layer*> Layers,QDate Start,
           QDate End,QString Title, QString Description, QVector<Task> Task);

    void AddTask  (long long Id);//Добавить задачу c id, равныи Id.
    void AddTask  (Task* NewTask);//Добавить задачу NewTask
    void AddTasks (QVector<long long> TasksId);//Добавить задачи c id TasksId
    void AddTasks (QVector<Task*> TasksPtr);//Добавить задачи, адресуемые указателями TasksPtr

    void DeleteTask  (long long Id);//Удалить задачу c id, равныи Id.
    void DeleteTask  (Task* DeletedTask);//Удалить задачу NewTask
    void DeleteTasks (QVector<long long> TasksId);//Удалить задачи c id TasksId
    void DeleteTasks (QVector<Layer*> TasksPtr);//Удалить задачи, адресуемые указателями TasksPtr

    virtual void Serialize  (QFile File);//Сериализация
    virtual void Deserialize(QFile File);//Десериализация

};

#endif // BASE_H

