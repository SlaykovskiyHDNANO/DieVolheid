#ifndef BASE_H
#define BASE_H
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QObject>
#include <QMultiMap>
#include <QJsonObject>
#include <QTime>

typedef QString idType;

class TimeSpan;
class Layer;
idType idGenerate();

class LayerAssistant : public QObject
{
    Q_OBJECT

    Layer *node;

    QMap <idType, Layer*> recursiveLayersMap;
    QMap <idType, TimeSpan*> recursiveTimeSpanMap;

    qint64 maxDepth;

    void recursiveDataGet (Layer* layer);
    void recursiveDepthCount (Layer* layer);
    void DepthCount (Layer* layer);

    LayerAssistant(const LayerAssistant&);
    LayerAssistant& operator= (const LayerAssistant&);
public:
    LayerAssistant(Layer* Node);
    void Recount();
    void Repick();
    qint64 GetMaxDepth();
    void AddLayer(Layer* layer);
    void AddTimeSpan(TimeSpan* ts);
    void DeleteLayer(idType id);
    void DeleteTimeSpan(idType id);
    Layer* GetLayer (idType id);
    TimeSpan* GetTimeSpan (idType id);
    QVector<Layer*> GetAllLayers ();
    QVector<TimeSpan*> GetAllTimeSpans ();
    //void Serialize  (QJsonObject &jsOb);
    //void Deserialize  (QJsonObject &jsOb);
};

class Layer : public QObject//слой
{
    Q_OBJECT

    friend class LayerAssistant;
    //Q_PROPERTY(idType idProp READ GetID WRITE SetID)
    //Q_PROPERTY(QString titleProp READ GetTitle WRITE SetTitle)
    //Q_PROPERTY(QString descriptionProp READ GetDescription WRITE SetDescription)

    idType           id;//В качестве id пока используется idType, затем он будет заменён
    QVector <TimeSpan*> intervals;//Вектор содержащие отрезки времени(события или задачи)
    Layer*              ParentLayer;//Родительский слой, то есть такой слой, для которого данный слой является подслоем
    QVector <Layer*>    sublayers;//подслои
    QString             title;//заголовок
    QString             description;//описание
    //qint64              maxDepth;//глубина поддерева слоев
    LayerAssistant*       assistant;//доп. информация о слое, подслоях, элементах

    void straightSublayersWalk(QVector<Layer *> &watchedLayers, int curDepth);
    void reverseSublayersWalk(QVector<Layer *> &watchedLayers, int curDepth);
    void lightLayerSerialization(QJsonObject &jsOb);
    void lightLayerDeserialization(QJsonObject &jsOb);
    void heavyLayerSerialization(QJsonObject &jsOb);
    void heavyLayerDeserialization(QJsonObject &jsOb);
    void lightRecursiveLayerSerialization(QJsonObject &jsOb, int curDepth);
    void lightRecursiveLayerDeserialization(QJsonObject &jsOb, int curDepth);
    void heavyRecursiveLayerSerialization(QJsonObject &jsOb, int curDepth);
    void heavyRecursiveLayerDeserialization(QJsonObject &jsOb, int curDepth);
    Layer* GetRoot(QVector<Layer*>& trace);


    Layer(const Layer&);
    Layer& operator= (const Layer&);
public:
    Layer ();
    Layer (idType Id, QVector<TimeSpan*> Intervals,QVector <Layer*> Sublayers, Layer* parentLayer,
           QString Title, QString Description/*, LayerAssistant* Assistant*/);

    idType          GetID();//Получить id
    QString            GetTitle();// Получить заголовок
    QString            GetDescription();//Получить описание
    Layer*             GetParrentLayer();//Получить родительский слой
    QVector<Layer*>    GetSublayers(int MaxDepth);//Получение всех подслоёв глубины не более MaxDepth
    QVector<TimeSpan*> GetAllIntervals();//Получение всех интервалов в слое, включая интервалы подслоёв,
    // интервалы подслоёв подслоёв и т.д. Причём полученный вектор содержит интервалы в порядке возрастания
    // даты начала и не содержит одинаковых интервалов

    void           SetID(idType id);
    void SetTitle       (QString NewTitle);// Сделать значением заголовка NewTitle
    void SetDescription (QString NewDescription);//  Сделать значением описания NewDescription
    void SetParentLayer (Layer* NewParent);//  Сделать родительским слоем NewParent
    void SetParentLayer (idType Id);// Сделать родительским слоем, слой с id равным Id

    void AddInterval  (idType Id);// Добавить в слой существующий интервал с id равным Id
    void AddInterval  (TimeSpan* Interval);// Добавить в слой  существующийинтервал Interval
    void NewInterval  (QDateTime Start, QDateTime End, QString IntervalTitle, QString IntervalDescription);
    //Создать новый интервал с датой начала Start, датой окончания End,
    //заголовком IntervalTitle,  описанием IntervalDescription.
    void AddIntervals (QVector<idType> IntervalsId);// Добавить интервалы с id IntervalsId
    void AddIntervals (QVector<TimeSpan*> IntervalsPtr);// Добавить интеревалы, адресуемые указателями IntervalsPtr

    void AddSublayer  (idType Id);// Сделать подслоем слой с id равным Id
    void AddSublayer  (Layer* Sublayer);//Сделать подслоем слой, адресуемый укзателем Sublayer
    void NewSublayer  (idType Id, QString SublayerTitle, QString SublayerDescription);
    //Создать новый подслой заголовком SublayerTitle,  описанием SublayerDescription.
    void AddSublayers (QVector<idType> SublayersId);//Добавить Добавить подслои с id SublayersId
    void AddSublayers (QVector<Layer*> SublayersPtr);// Добавить подслои, адресуемые указателями SublayersPtr

    void DeleteInterval  (idType Id);// Удалить из слоя интервал с id равным Id
    void DeleteInterval  (TimeSpan* Interval);// Удалить из слоя интервал, адресуемый укзателем Interval
    void DeleteIntervals (QVector<idType> IntervalsId);// Удалить из слоя интервалы с id IntervalsId
    void DeleteIntervals (QVector<TimeSpan*> IntervalsPtr);// Удалить из слоя интеревалы, адресуемые указателями IntervalsPtr

    void DeleteSublayer  (idType Id);// Удалить подслой с id равным Id
    void DeleteSublayer  (Layer* Sublayer);// Удалить подслой, адресуемый укзателем Sublayer
    void DeleteSublayers (QVector<idType> SublayersId);//Удалить подслои с id SublayersId
    void DeleteSublayers (QVector<Layer*> SublayersPtr);// Удалить подслои, адресуемые указателями SublayersPtr

    void Clear();//Очитка слоя, т.е. удаление из него всех подслоёв,интервалов, заголовка, описания.

    enum typeOfSerialization {LIGHT_SERIALIZATION, HEAVY_SERIALIZATION};
    enum typeOfDeserialization {LIGHT_DESERIALIZATION, HEAVY_DESERIALIZATION};
    void Serialize(QJsonObject &jsOb, int depth, Layer::typeOfSerialization ts);//Сериализация
    void Deserialize(QJsonObject &jsOb, int depth, Layer::typeOfDeserialization td);//Десериализация
};

class TimeSpan : public QObject//Временной промежуток
{
    Q_OBJECT

    //Q_PROPERTY(idType idProp READ GetID WRITE SetID)
    //Q_PROPERTY(QDateTime startProp READ GetStart WRITE SetStart)
    //Q_PROPERTY(QDateTime endProp READ GetEnd WRITE SetEnd)
    //Q_PROPERTY(QString titleProp READ GetTitle WRITE SetTitle)
    //Q_PROPERTY(QString descriptionProp READ GetDescription WRITE SetDescription)

    idType        id;//В качестве id пока используется idType, затем он будет заменён
    QVector <Layer*> layers;//слои, связанные с промежутком времени
    QDateTime            start;//Начала интервала
    QDateTime            end;//Конец интервала
    QString          title;//заголовок
    QString          description;//описание

public:
    TimeSpan();
    TimeSpan (idType Id, QVector<Layer*> Layers,QDateTime Start,
           QDateTime End,QString Title, QString Description);

    idType        GetID();//Получить id
    QString          GetTitle();// Получить заголовок
    QString          GetDescription();//Получить описание
    QDateTime            GetStart();//Получить начало отрезка
    QDateTime            GetEnd();//Получить конец отрезка
    QVector <Layer*> GetLayers();// Получить, связанные с промежутком слои

    void        SetID(idType id);
    void SetTitle(QString NewTitle);//  Сделать значением заголовка NewTitle
    void SetDescription(QString NewDescription);//  Сделать значением описания NewDescription
    void SetStart(QDateTime NewStart);//  Сделать значением начала отрезка NewStart
    void SetEnd(QDateTime NewEnd);    //  Сделать значением конца отрезка NewEnd
    void SetLayers(QVector <Layer*> Layers);// Получить, связанные с промежутком слои

    void AddLayer  (idType Id);//Добавить слой c id, равныи Id.
    void AddLayer  (Layer* NewLayer);//Добавить слой NewLayer
    void AddLayers (QVector<idType> LayersId);//Добавить слои c id LayersId
    void AddLayers (QVector<Layer*> LayersPtr);//Добавить слои, адресуемые указателями LayersPtr

    void DeleteLayer  (idType Id);//Удалить слой c id, равныи Id.
    void DeleteLayer  (Layer* DeletedLayer);//Удалить слой NewLayer
    void DeleteLayers (QVector<idType> LayersId);//Удалить слои c id LayersId
    void DeleteLayers (QVector<Layer*> LayersPtr);//Удалить слои, адресуемые указателями LayersPtr

    virtual void Serialize  (QJsonObject &jsOb);//Сериализация
    virtual void Deserialize(QJsonObject &jsOb);//Десериализация
};
class Task: public TimeSpan//Задача
{
    Q_OBJECT

    //Q_PROPERTY(bool isPerformedProp READ GetIsPerformed WRITE SetIsPerformed)

    bool is_performed;// выполнена?
public:
    Task();
    Task (idType Id, QVector<Layer*> Layers,QDateTime Start,
           QDateTime End,QString Title, QString Description, bool IsPerformed);

    bool GetIsPerformed();
    void SetIsPerformed(bool IsPerformed);

    virtual void Serialize  (QJsonObject& jsOb);//Сериализация
    virtual void Deserialize(QJsonObject& jsOb);//Десериализация

};


class Event: public TimeSpan//Событие
{
    Q_OBJECT
    QVector<Task> tasks;//Задачи, входящие в событие
public:
    Event();
    Event (idType Id, QVector<Layer*> Layers,QDateTime Start,
           QDateTime End,QString Title, QString Description, QVector<Task> Task);

    void AddTask  (idType Id);//Добавить задачу c id, равныи Id.
    void AddTask  (Task* NewTask);//Добавить задачу NewTask
    void AddTasks (QVector<idType> TasksId);//Добавить задачи c id TasksId
    void AddTasks (QVector<Task*> TasksPtr);//Добавить задачи, адресуемые указателями TasksPtr

    void DeleteTask  (idType Id);//Удалить задачу c id, равныи Id.
    void DeleteTask  (Task* DeletedTask);//Удалить задачу NewTask
    void DeleteTasks (QVector<idType> TasksId);//Удалить задачи c id TasksId
    void DeleteTasks (QVector<Layer*> TasksPtr);//Удалить задачи, адресуемые указателями TasksPtr

    virtual void Serialize  (QJsonObject &jsOb);//Сериализация
    virtual void Deserialize(QJsonObject &jsOb);//Десериализация

};

#endif // BASE_H

