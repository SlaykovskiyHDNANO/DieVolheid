#ifndef BASE_H
#define BASE_H
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QTime>

typedef QString idType;

class User;

class GlobalAssistant;
class GlobalAssistantSerializer;
class GlobalAssistantDeserializer;

class Layer;
class LayerSerializer;
class LayerDeserializer;

class TimeSpan;
class TimeSpanSerializer;
class TimeSpanDeserializer;

class Task;
class TaskSerializer;
class TaskDeserializer;

class Event;
class EventSerializer;
class EventDeserializer;




class User
{
    GlobalAssistant* gs; //ассистент пользователя
    QVector<idType> forest; //id корневых вершин
public:
    User(); //создает ассистента

    /***********************************************************TimeSpan**************************************************************/
    //создает промежуток и возвращает его id
    idType CreateTimeSpan(QVector<idType> Layers, //в каких слоях находится промежуток
                          QDateTime Start, //начало
                          QDateTime End, //конец
                          QString Title, //заголовок
                          QString Description); //описание

    bool DeleteTimeSpan(idType id); //удаляет промежуток по id

    QString                GetTitleOfTimeSpan(idType id);          //Получить заголовок интервала по id
    QString                GetDescriptionOfTimeSpan(idType id);    //Получить описание  интервала по id
    QDateTime              GetStartOfTimeSpan(idType id);          //Получить начало отрезка интервала по id
    QDateTime              GetEndOfTimeSpan(idType id);            //Получить конец отрезка интервала по id
    QVector <idType>       GetLayersOfTimeSpan(idType id);         //Получить, связанные с промежутком слои по id
    QVector <idType>       GetAllTimeSpans(idType id);             //Получить id всех промежутков по id

    void SetTitleOfTimeSpan(idType id, QString NewTitle);                    //Сделать значением заголовка NewTitle у промежутка с id
    void SetDescriptionOfTimeSpan(idType id, QString NewDescription);        //Сделать значением описания NewDescription у промежутка с id
    void SetStartOfTimeSpan(idType id, QDateTime NewStart);                  //Сделать значением начала отрезка NewStart у промежутка с id
    void SetEndOfTimeSpan(idType id, QDateTime NewEnd);                      //Сделать значением конца отрезка NewEnd у промежутка с id
    void SetLayersOfTimeSpan(idType id, QVector <idType> Layers);            //Установить связанные с промежутком слои у промежутка с id

    /***********************************************************Layer**************************************************************/
    idType CreateLayer(QVector<idType> Intervals,
                       QVector<idType> Sublayers,
                       idType ParentLayer,
                       QString Title,
                       QString Description);

    QString          GetTitleOfLayer(idType id);            //Получить заголовок слоя по id слоя
    QString          GetDescriptionOfLayer(idType id);      //Получить описание слоя по id слоя
    idType           GetParentLayerOfLayer(idType id);      //Получить id родительского слоя слоя по id слоя
    QVector<idType>  GetSublayersOfLayer(idType id);        //Получение всех подслоёв только этого слоя слоя по id слоя
    QVector<idType>  GetRecSublayersOfLayer(idType id);     //Получение всех подслоёв рекурсивно слоя по id слоя
    QVector<idType>  GetAllIntervalsOfLayer(idType id);     //Получение всех интервалов в слое слоя по id слоя
    QVector<idType>  GetRecAllIntervalsOfLayer(idType id);  //Получение всех интервалов в слое, включая интервалы подслоёв,
                                                            //интервалы подслоёв подслоёв и т.д. Причём полученный вектор
                                                            //содержит интервалы в порядке возрастания
                                                            //даты начала и не содержит одинаковых интервалов слоя по id слоя

    void SetTitleOfLayer           (idType Id, QString NewTitle);         //Сделать значением заголовка NewTitle по id слоя
    void SetDescriptionOfLayer     (idType Id, QString NewDescription);   //Сделать значением описания NewDescription по id слоя
    void SetParentLayerOfLayer     (idType Id, idType IdParent);          //Сделать родительским слоем, слой с id равным IdParent по id слоя

    void AddIntervalToLayer            (idType Id, idType IdOfInterval);                      //Добавить в слой существующий интервал с id равным IdInterval по id слоя
    void AddIntervalsToLayer           (idType Id, QVector<idType> IntervalsId);              //Добавить интервалы с id IntervalsId в слой по id слоя
    void NewIntervalToLayer            (idType Id,
                                        QDateTime Start,
                                        QDateTime End,
                                        QString IntervalTitle,
                                        QString IntervalDescription);    //Создать новый интервал с датой начала Start, датой окончания End,
                                                                         //заголовком IntervalTitle,  описанием IntervalDescription  по id слоя

    void AddSublayerToLayer            (idType Id, idType IdOfSublayer); // Сделать подслоем слой с id равным IdSublayer по id слоя
    void NewSublayerToLayer            (idType Id,
                                        QString SublayerTitle,
                                        QString SublayerDescription);
                                                                  //Создать новый подслой заголовком SublayerTitle,
                                                                  //описанием SublayerDescription  по id слоя
    void AddSublayersToLayer (idType Id, QVector<idType> SublayersId);   //Добавить подслои с id SublayersId в слой по id слоя

    void DeleteIntervalFromLayer  (idType Id, idType IdOfInterval);         // Удалить из слоя интервал с id равным IdOfInterval по id слоя
    void DeleteIntervalsFromLayer (idType Id, QVector<idType> IntervalsId); // Удалить из слоя интервалы с id IntervalsId

    void DeleteSublayerFromLayer  (idType Id, idType IdOfSublayer);         // Удалить подслой с id равным IdOfSublayer по id слоя
    void DeleteSublayersFromLayer (idType Id, QVector<idType> SublayersId); //Удалить подслои с id SublayersId по id слоя
    /***********************************************************Task**************************************************************/
    /***********************************************************Event**************************************************************/

    //удаляет ассистента
   ~User();
};

//ведёт учёт всех слоёв, всех промежутков времени, глубины вложенности
//класс требуется, в основном, для централизованного хранения указателей на имеющиеся объекты и , соответственно,
//поддержания механизмов обращения по id, предоставления быстрого доступа к любому элементу,
//предотвращения множественного появления в памяти "клонов" объектов (в частности, при десериализации, когда один объект
//хранился во множестве слоев). Т.е. основная идея - чем больше всё выражени на языке id, тем лучше, данный класс отвечает за это.
class GlobalAssistant : public QObject
{
    Q_OBJECT

    QMap <idType, Layer*>       LayersMap;      //хранит указатели на все предоставленные слои
    QMap <idType, TimeSpan*>    TimeSpanMap;    //хранит указатели на все предоставленные промежутки
    QMap <idType, qint64>       depthMap;       //хранит высоту дерева с корнем в узле с указанным id
    //(в QMap одному ключу соответствует одно значение, повторений не будет)

    void recursiveDepthCount (idType l);    //рекурсивно пересчитывает высоту переданного слоя и всех его подслоев
    void DepthCount          (idType l);    //вспомогательная функция для recursiveDepthCount
                                            //определяет высоту переданного слоя как макс. высоту его подслоёв + 1
    //не надо этого делать =)
    GlobalAssistant(const GlobalAssistant&);
    GlobalAssistant& operator= (const GlobalAssistant&);
public:

    static idType idGenerate(); //генерирует id

    GlobalAssistant(); //ничего не делает

    void Recount   (idType id);                    //пересчитать высоту слоя с id
    void RecountAll(QVector<idType> Layers);       //пересчитать высоты всех переданных слоев

    void ConnectToLayer (Layer* l);                //подключиться к слою (установить в нём указатель на ассистента на себя)
    void ConnectToLayers(QVector<Layer*> Layers);  //подключиться к каждому переданному слою

    void AddLayer       (Layer* layer);    //добавить слой
    void AddTimeSpan    (TimeSpan* ts);    //добавить интервал
    void DeleteLayer    (idType id);       //удалить слой по id
    void DeleteTimeSpan (idType id);       //удалить интервал по id

    Layer*              GetLayer (idType id);            //получить слой по id
    TimeSpan*           GetTimeSpan (idType id);         //получить интервал по id
    QVector<Layer*>     GetAllLayers ();                 //получить все слои
    QVector<TimeSpan*>  GetAllTimeSpans ();              //получить все промежутки
    qint64              GetDepth(idType id);             //получить глубину слоя по id
    void                SetDepth(idType id, qint64 val); //установить глубину слоя

    void Serialize    (QJsonObject &jsOb);  //сериализация
    void Deserialize  (QJsonObject &jsOb);  //десериализация

    ~GlobalAssistant(); //деструктор очищает все хранимые указатели
};

//сериализатор для GlobalAssistant
class GlobalAssistantSerializer
{
public:
    static void Serialize(QJsonObject &jsOb, GlobalAssistant* gs);
};

//десериализатор для GlobalAssistant
class GlobalAssistantDeserializer
{
public:
    static void Deserialize(QJsonObject &jsOb, GlobalAssistant* gs);
};


/********************************************************
 * Layer
 * ******************************************************/
class Layer : public QObject//слой
{
    Q_OBJECT

    idType              id;             //В качестве id пока используется idType, затем он будет заменён
    QVector <idType>    intervals;      //Вектор содержащий id отрезков времени(события или задачи)
    idType              parentLayer;    //id родительского слоя, то есть такого слоя, для которого данный слой является подслоем
    QVector <idType>    sublayers;      //id подслоев
    QString             title;          //заголовок
    QString             description;    //описание

    GlobalAssistant*    assistant;//информация о самом слое и окружающих слоях, их содержимом

    void straightSublayersWalk(QVector<idType> &watchedLayers); //обойти все подслои в прямом порядке и вернуть их id
    void reverseSublayersWalk(QVector<idType> &watchedLayers);  // обойти все подслои в обратном порядке и вернуть их id

    //лучше так не делать =)
    Layer(const Layer&);
    Layer& operator= (const Layer&);

public:

    Layer (); //только зануляет указатель на ассистента и устанавливает id
    Layer (QVector<idType> Intervals,   //интервалы слоя
           QVector <idType> Sublayers,  //подслои слоя
           idType ParentLayer,          //родительский слой
           QString Title,               //заголовок
           QString Description,         //описание
           GlobalAssistant* Assistant); //указатель на ассистента

    idType           GetID();               //Получить id
    QString          GetTitle();            //Получить заголовок
    QString          GetDescription();      //Получить описание
    idType           GetParentLayer();      //Получить id родительского слоя
    QVector<idType>  GetSublayers();        //Получение всех подслоёв только этого слоя
    QVector<idType>  GetRecSublayers();     //Получение всех подслоёв рекурсивно
    QVector<idType>  GetAllIntervals();     //Получение всех интервалов в слое
    QVector<idType>  GetRecAllIntervals();  //Получение всех интервалов в слое, включая интервалы подслоёв,
                                            //интервалы подслоёв подслоёв и т.д. Причём полученный вектор
                                            //содержит интервалы в порядке возрастания
                                            //даты начала и не содержит одинаковых интервалов
    GlobalAssistant* GetGlobalAssistant();  //получить указатель на ассистента

    void SetID              (idType Id);                //установить Id слоя
    void SetTitle           (QString NewTitle);         //Сделать значением заголовка NewTitle
    void SetDescription     (QString NewDescription);   //Сделать значением описания NewDescription
    void SetParentLayer     (idType Id);                //Сделать родительским слоем, слой с id равным Id
    void SetGlobalAssistant (GlobalAssistant* gas);     //установить глобального ассистента

    void AddInterval            (idType Id);                      //Добавить в слой существующий интервал с id равным Id
    void AddIntervalStupidly    (idType Id);                      //просто добавить id в интервалы
    void AddIntervals           (QVector<idType> IntervalsId);    //Добавить интервалы с id IntervalsId
    void NewInterval            (QDateTime Start,
                                 QDateTime End,
                                 QString IntervalTitle,
                                 QString IntervalDescription);    //Создать новый интервал с датой начала Start, датой окончания End,
                                                                  //заголовком IntervalTitle,  описанием IntervalDescription.

    void AddSublayer            (idType Id);                      // Сделать подслоем слой с id равным Id
    void AddSublayerStupidly    (idType Id);                      //просто добавить id в подслои
    void NewSublayer            (QString SublayerTitle,
                                 QString SublayerDescription);
                                                                  //Создать новый подслой заголовком SublayerTitle,
                                                                  //описанием SublayerDescription.
    void AddSublayers (QVector<idType> SublayersId);              //Добавить подслои с id SublayersId

    void DeleteInterval  (idType Id);                   // Удалить из слоя интервал с id равным Id
    void DeleteIntervals (QVector<idType> IntervalsId); // Удалить из слоя интервалы с id IntervalsId

    void DeleteSublayer  (idType Id);// Удалить подслой с id равным Id
    void DeleteSublayers (QVector<idType> SublayersId);//Удалить подслои с id SublayersId

    void Clear();//Очитка слоя, т.е. удаление из него всех подслоёв,интервалов, заголовка, описания.

    void Serialize              (QJsonObject &jsOb);    //Сериализация
    void Deserialize            (QJsonObject &jsOb);    //Десериализация
    void SerializeRecursively   (QJsonObject &jsOb);    //рекурсивная сериализация
    void DeserializeRecursively (QJsonObject &jsOb);    //рекурсивная десериализация
};

//сериализатор для слоя
class LayerSerializer
{
public:
    static void Serialize               (QJsonObject &jsOb, Layer* l);
    static void SerializeRecursively    (QJsonObject &jsOb, Layer* l);
};

//десериализатор для слоя
class LayerDeserializer
{
public:
    static void Deserialize               (QJsonObject &jsOb, Layer* l);
    static void DeserializeRecursively    (QJsonObject &jsOb, Layer *l);
};

/********************************************************
 * TimeSpan
 * ******************************************************/

class TimeSpan : public QObject     //Временной промежуток
{
    Q_OBJECT

    idType              id;         //В качестве id пока используется idType, затем он будет заменён
    QVector <idType>    layers;     //id слоев, связанные с промежутком времени
    QDateTime           start;      //Начало интервала
    QDateTime           end;        //Конец интервала
    QString             title;      //заголовок
    QString             description;//описание

    GlobalAssistant* assistant;     //ассистент - хранит указатель на промежуток, информацию о слоях, связанных с промежутком

public:
   TimeSpan(); //ничего не делает
   TimeSpan (QVector<idType> Layers,        //в каких слоях хранится
              QDateTime Start,              //начало
              QDateTime End,                //конец
              QString Title,                //заголовок
              QString Description,          //описание
              GlobalAssistant* Assistant    //ассистент для промежутка
              );

    idType                 GetID();             //Получить id
    QString                GetTitle();          //Получить заголовок
    QString                GetDescription();    //Получить описание
    QDateTime              GetStart();          //Получить начало отрезка
    QDateTime              GetEnd();            //Получить конец отрезка
    QVector <idType>       GetLayers();         //Получить, связанные с промежутком слои
    GlobalAssistant*       GetGlobalAssistant();//Получить указатель на ассистента

    void SetID(idType id);                              //установить id
    void SetTitle(QString NewTitle);                    //Сделать значением заголовка NewTitle
    void SetDescription(QString NewDescription);        //Сделать значением описания NewDescription
    void SetStart(QDateTime NewStart);                  //Сделать значением начала отрезка NewStart
    void SetEnd(QDateTime NewEnd);                      //Сделать значением конца отрезка NewEnd
    void SetLayers(QVector <idType> Layers);            //Установить связанные с промежутком слои
    void SetGlobalAssistant(GlobalAssistant* Assistant);//Установить ассистента

    void AddLayer  (idType Id);                 //Добавить слой c id, равныи Id.
    void AddLayers (QVector<idType> LayersId);  //Добавить слои c id LayersId

    void DeleteLayer  (idType Id);                  //Удалить слой c id, равныи Id
    void DeleteLayers (QVector<idType> LayersId);   //Удалить слои c id LayersId

    virtual void Serialize  (QJsonObject &jsOb);//Сериализация
    virtual void Deserialize(QJsonObject &jsOb);//Десериализация
};

//сериализатор для TimeSpan
class TimeSpanSerializer
{
public:
    static void Serialize(QJsonObject &jsOb, TimeSpan* ts);
};

//десериализатор для TimeSpan
class TimeSpanDeserializer
{
public:
    static void Deserialize(QJsonObject &jsOb, TimeSpan* ts);
};

/********************************************************
 * Task
 * ******************************************************/

class Task: public TimeSpan//Задача
{
    Q_OBJECT

    bool is_performed;// выполнена?
public:
    Task(); //ничего не делает
    Task (QVector<idType> Layers,       //массив id связанных слоев
          QDateTime Start,              //время начала
          QDateTime End,                //время конца
          QString Title,                //заголовок
          QString Description,          //описание
          GlobalAssistant* Assistant,   //указатель на ассистента
          bool IsPerformed);            //выполнена ли

    bool GetIsPerformed();                  //выполнена ли задача
    void SetIsPerformed(bool IsPerformed);  //установть состояние выполнения

    virtual void Serialize  (QJsonObject& jsOb);//Сериализация
    virtual void Deserialize(QJsonObject& jsOb);//Десериализация

};

//сериализатор для Task
class TaskSerializer
{
public:
    static void Serialize(QJsonObject &jsOb, Task* ts);
};

//десериализатор для Task
class TaskDeserializer
{
public:
    static void Deserialize(QJsonObject &jsOb, Task* ts);
};

/********************************************************
 * Event
 * ******************************************************/

class Event: public TimeSpan//Событие
{
    Q_OBJECT
    QVector<idType> tasks;//Задачи, входящие в событие
public:
    Event(); //ничего не делает
    Event (QVector<idType> Layers,      //в каких слоях событие
           QDateTime Start,             //время начала
           QDateTime End,               //время конца
           QString Title,               //зоголовок
           QString Description,         //описание
           GlobalAssistant* Assistant,  //указатель на ассистента
           QVector<idType> Tasks);        //массив задач, входящих в событие

    QVector<idType> GetAllTasks(); //получить все задачи, связанные с событием

    void AddTask  (idType Id);              //Добавить задачу c id, равныи Id.
    void AddTasks (QVector<idType> TasksId);//Добавить задачи c id TasksId

    void DeleteTask  (idType Id);               //Удалить задачу c id, равныи Id.
    void DeleteTasks (QVector<idType> TasksId); //Удалить задачи c id TasksId

    virtual void Serialize  (QJsonObject &jsOb);//Сериализация
    virtual void Deserialize(QJsonObject &jsOb);//Десериализация

};

//сериализатор для Event
class EventSerializer
{
public:
    static void Serialize(QJsonObject &jsOb, Event* ts);
};

//десериализатор для Event
class EventDeserializer
{
public:
    static void Deserialize(QJsonObject &jsOb, Event* ts);
};

#endif // BASE_H

