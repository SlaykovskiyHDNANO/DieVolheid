#include "base.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>


/*********************************************
idGenerate
**********************************************/
idType GlobalAssistant::idGenerate()
{
    QTime t(0, 0, 0);
    qsrand(t.secsTo(QTime::currentTime())); //иниц. генератор
    idType id;
    for (int i = 0; i < 10; ++i) //получить строку из 10 случайных символов, любых, кроме 0
                                 //всего таких строк 254^10 > 10^24
    {
        int tmp = qrand() % 254 + 1;
        id.append( (char)tmp );
    }
    id.append(0); //завершающий 0
    return id;
}

/*********************************************
Timespan
**********************************************/

void TimeSpan::SetID(idType Id)
{
    this->id = Id;
}

TimeSpan::TimeSpan() : QObject()
{

}

TimeSpan::TimeSpan(QVector<idType> Layers, QDateTime Start, QDateTime End, QString Title, QString Description, GlobalAssistant *Assistant)
{
    SetGlobalAssistant(Assistant); //установить ассистента
    SetID(GlobalAssistant::idGenerate()); //сгенерировать id
    SetStart(Start); //установить начало
    SetEnd(End); //установить конец
    SetTitle(Title); //установить заголовок
    SetDescription(Description); //установить описание
    SetLayers(Layers); //установить связанные слои
}

idType TimeSpan::GetID()
{
    return this->id;
}

QString TimeSpan::GetTitle()
{
    return this->title;
}

QString TimeSpan::GetDescription()
{
    return this->description;
}

QDateTime TimeSpan::GetStart()
{
    return this->start;
}

QDateTime TimeSpan::GetEnd()
{
    return this->end;
}

QVector<idType> TimeSpan::GetLayers()
{
    return this->layers;
}

GlobalAssistant *TimeSpan::GetGlobalAssistant()
{
    return assistant;
}

void TimeSpan::SetTitle(QString NewTitle)
{
    this->title = NewTitle;
}

void TimeSpan::SetDescription(QString NewDescription)
{
    this->description = NewDescription;
}

void TimeSpan::SetStart(QDateTime NewStart)
{
    this->start = NewStart;
}

void TimeSpan::SetEnd(QDateTime NewEnd)
{
    this->end = NewEnd;
}

void TimeSpan::SetLayers(QVector<idType> Layers)
{
    this->layers = Layers;
    foreach (idType idL, Layers) { //добавляем промежуток к каждому слою
        AddLayer(idL);
    }
}

void TimeSpan::SetGlobalAssistant(GlobalAssistant *Assistant)
{
    assistant = Assistant;
}

void TimeSpan::AddLayer(idType Id)
{
    //получаем указатели на слой у ассистента и добавляем к нему интервал
    assistant->GetLayer(Id)->AddInterval(GetID());
}

void TimeSpan::AddLayers(QVector<idType> LayersId)
{
    foreach (idType l, LayersId) {
        AddLayer(l);
    }
}

void TimeSpan::DeleteLayer(idType Id)
{
    layers.removeOne(Id);
    assistant->GetLayer(Id)->DeleteInterval(GetID());
}

void TimeSpan::DeleteLayers(QVector<idType> LayersId)
{
    foreach (idType idL, LayersId) {
        DeleteLayer(idL);
    }
}

void TimeSpan::Serialize(QJsonObject& jsOb)
{
    TimeSpanSerializer::Serialize(jsOb, this);
}

void TimeSpan::Deserialize(QJsonObject& jsOb)
{
    TimeSpanDeserializer::Deserialize(jsOb, this);
}

/***************************************************/

/**************************************************
Task
**************************************************/

Task::Task() : TimeSpan()
{

}

Task::Task(QVector<idType> Layers,
           QDateTime Start,
           QDateTime End,
           QString Title,
           QString Description,
           GlobalAssistant *Assistant,
           bool IsPerformed):
    TimeSpan(Layers, Start, End, Title, Description, Assistant)
{
    SetIsPerformed(IsPerformed);
}

bool Task::GetIsPerformed()
{
    return is_performed;
}

void Task::SetIsPerformed(bool IsPerformed)
{
    is_performed = IsPerformed;
}

void Task::Serialize(QJsonObject &jsOb)
{
    TaskSerializer::Serialize(jsOb, this);
}

void Task::Deserialize(QJsonObject &jsOb)
{
    TaskDeserializer::Deserialize(jsOb, this);
}
/***************************************************/

/**************************************************
Event
**************************************************/

Event::Event() : TimeSpan()
{

}

Event::Event(QVector<idType> Layers,
             QDateTime Start,
             QDateTime End,
             QString Title,
             QString Description,
             GlobalAssistant *Assistant,
             QVector<idType> Tasks):
    TimeSpan(Layers, Start, End, Title, Description, Assistant)
{
    AddTasks(Tasks);
}

QVector<idType> Event::GetAllTasks()
{
    return tasks;
}

void Event::AddTask(idType Id)
{
    TimeSpan* tsPtr = GetGlobalAssistant()->GetTimeSpan(Id); //получаем добавляемую задачу у ассистента
    Task* tskToAdd;//указатель на задачу, которую надо добавить
    if(!tsPtr) //если такого промежутка нет - вернуться
        return;
    if(tsPtr->metaObject()->className() == Task::QObject::staticMetaObject.className()) //если у промежутка тип зпдпчи
    {
        tskToAdd = (Task*) tsPtr; //привести к указателю на задачу
    }
    else// иначе вернуться
        return;
    if(!tasks.contains(Id)) //если такая задача есть у ассистента, но ее еще нет в списке задач данного события
    {
        tasks.append(Id); //то мы ее добавляем
    }
}

void Event::AddTasks(QVector<idType> TasksId)
{
    foreach (idType idTsk, TasksId) {
        AddTask(idTsk);
    }
}

void Event::DeleteTask(idType Id)
{
    if(tasks.contains(Id)) //если такая задача есть
        tasks.removeOne(Id);
}

void Event::DeleteTasks(QVector<idType> TasksId)
{
    foreach (idType idTsk, TasksId) {
        DeleteTask(idTsk);
    }
}

void Event::Serialize(QJsonObject &jsOb)
{
    EventSerializer::Serialize(jsOb, this);
}

void Event::Deserialize(QJsonObject &jsOb)
{
    EventDeserializer::Deserialize(jsOb, this);
}

/*********************************************
Layer
**********************************************/

void Layer::straightSublayersWalk(QVector<idType> &watchedLayers)
{
    watchedLayers.append(this->GetID()); //добавить id текущего слоя
    if(sublayers.empty()) //если нет подслоев - вернуться
        return;
    foreach (idType l, sublayers) { //обойти все подслои в глубину
        assistant->GetLayer(l)->straightSublayersWalk(watchedLayers);
    }
}

void Layer::reverseSublayersWalk(QVector<idType> &watchedLayers)
{
    if (sublayers.empty()) //если глубже идти не надо
        watchedLayers.append(this->GetID()); //добавить id текущего слоя и вернуться
        return;
    foreach (idType l, sublayers) { //иначе обойти все подслои
        assistant->GetLayer(l)->reverseSublayersWalk(watchedLayers);
    }
    watchedLayers.append(this->GetID()); //затем добавить текущий слой
}

Layer::Layer() : QObject()
{
    SetGlobalAssistant(0);
    SetID(GlobalAssistant::idGenerate());
}

Layer::Layer(QVector<idType> Intervals,
             QVector<idType> Sublayers,
             idType ParentLayer,
             QString Title,
             QString Description,
             GlobalAssistant *Assistant):
QObject()
{
    SetGlobalAssistant(Assistant);          //устанавливаем ассистента сначала (он нужен для установки последующих параметров)
    SetID(GlobalAssistant::idGenerate());   //устанавливаем id
    SetParentLayer(ParentLayer);            //устанавливаем родительский слой
    SetTitle(Title);                        //устанавливаем заголовок
    SetDescription(Description);            //устанавливаем описание
    AddIntervals(Intervals);                //добавляем интервалы
    AddSublayers(Sublayers);                //добавляем подслои
}

idType Layer::GetID()
{
    return this->id;
}

QString Layer::GetTitle()
{
    return this->title;
}

QString Layer::GetDescription()
{
    return this->description;
}

idType Layer::GetParentLayer()
{
    return parentLayer;
}

QVector<idType> Layer::GetSublayers()
{
    return sublayers;
}

QVector<idType> Layer::GetRecSublayers()
{
    QVector<idType> Sublayers; //массив с подслоями
    straightSublayersWalk(Sublayers); //получаем все подслои прямым обходом
    return Sublayers;
}

QVector<idType> Layer::GetAllIntervals()
{
    return intervals;
}

QVector<idType> Layer::GetRecAllIntervals()
{
     QVector<idType> Sublayers = GetRecSublayers(); //получение рекурсивно всех подслоев
     QSet<idType> Intervals; //множество с интервалами (нет повторений)
     foreach (idType idL, Sublayers) { //для каждого подслоя
         QVector<idType> idTs = assistant->GetLayer(idL)->GetAllIntervals(); //получить интервалы из подслоя
         foreach (idType id, idTs) { //для всех интервалов текущего подслоя
             Intervals.insert(id); //втавить в множество id интервала
         }//foreach
     }//foreach
     return Intervals.toList().toVector();
}

GlobalAssistant *Layer::GetGlobalAssistant()
{
    return assistant;
}

void Layer::SetID(idType Id)
{
    id = Id;
}

void Layer::SetTitle(QString NewTitle)
{
    this->title = NewTitle;
}

void Layer::SetDescription(QString NewDescription)
{
    this->description = NewDescription;
}

void Layer::SetParentLayer(idType NewParent)
{
    this->parentLayer = NewParent;
}

void Layer::SetGlobalAssistant(GlobalAssistant *gas)
{
    this->assistant = gas;
}

void Layer::AddInterval(idType Id)
{
    if(!intervals.contains(Id)) //если этого интервала ещё нет в этом слое
    {
        TimeSpan* ts = assistant->GetTimeSpan(Id); //получаем интервал у ассистента
        if(ts) //если такой интервал существует
        {
            intervals.append(Id); //добавляем id интервала в слой
            ts->AddLayer(this->GetID()); //к полученному интервалу добавляем слой
        }
    }
}

void Layer::AddIntervalStupidly(idType Id)
{
    intervals.append(Id);
}//fnc

void Layer::NewInterval(QDateTime Start,
                        QDateTime End,
                        QString IntervalTitle,
                        QString IntervalDescription)
{
    TimeSpan *pts = new TimeSpan(QVector<idType>(0), //пока слоев с этим интервалом нет
                                 Start,
                                 End,
                                 IntervalTitle,
                                 IntervalDescription,
                                 GetGlobalAssistant());
    GetGlobalAssistant()->AddTimeSpan(pts); //регистрируем новый промежуток у ассистента
    AddInterval(pts->GetID()); //добавить интервал к слою
}

void Layer::AddIntervals(QVector<idType> IntervalsId)
{
    foreach (idType id, IntervalsId) {
        AddInterval(id);
    }
}

void Layer::AddSublayer(idType Id)
{
    if (!sublayers.contains(Id)) //если такого подслоя нет
    {
        Layer* LayerToAdd = assistant->GetLayer(Id); //получаем этот слой у ассистента
        if(LayerToAdd) //если такой слой есть
        {
            sublayers.append(Id); //добавляем к подслоям текущего слоя
            LayerToAdd->SetParentLayer(GetID()); //устанавливаем предком  добавленному слою текущий слой
        }//if
    }//if
}

void Layer::AddSublayerStupidly(idType Id)
{
    sublayers.append(Id);
}

void Layer::NewSublayer(QString SublayerTitle, QString SublayerDescription)
{
    Layer* l = new Layer(QVector<idType>(0), //пока у нового слоя нет интервалов
                         QVector<idType>(0), //пока у нового слоя нет подслоев
                         GetID(), //id родительского слоя - id текущего слоя
                         SublayerTitle, //заголовок
                         SublayerDescription, //описание
                         GetGlobalAssistant()); //ассистент
    GetGlobalAssistant()->AddLayer(l); //регистрируем слой у ассистента
    AddSublayer(l->GetID()); //добавляем его в качестве подслоя
}

void Layer::AddSublayers(QVector<idType> SublayersId)
{
    foreach (idType id, SublayersId) {
        AddSublayer(id);
    }
}

void Layer::DeleteInterval(idType Id)
{
    if(intervals.contains(Id)) //если интервал присутствует в слое
    {
        TimeSpan* ts = GetGlobalAssistant()->GetTimeSpan(Id);
        if(ts) //если такой интервал существует
        {
            intervals.removeOne(Id); //удаляем интервал из слоя
            ts->DeleteLayer(GetID()); //удаляем слой из интервала
        }//if
    }//if
}

void Layer::DeleteIntervals(QVector<idType> IntervalsId)
{
    foreach (idType i,IntervalsId) {
        DeleteInterval(i);
    }
}

void Layer::DeleteSublayer(idType Id)
{
    if (sublayers.contains(Id)) //если в подслоях присутствует такой слой
    {
        Layer* l = GetGlobalAssistant()->GetLayer(Id);
        if(l) //если такой слой существует
        {
            sublayers.removeOne(Id); //удаляем слой из подслоев
            l->SetParentLayer(idType("0")); //родителя у удаленного подслоя теперь нет
        }//if
    }//if
}

void Layer::DeleteSublayers(QVector<idType> SublayersId)
{
    foreach (idType i, SublayersId) {
        DeleteSublayer(i);
    }
}

void Layer::Clear()
{
    foreach (idType idL, sublayers) { //сначала рекурсивно очистить все подслои
        Layer* l = GetGlobalAssistant()->GetLayer(idL); //получить указатель на удаляемй слой
        l->Clear(); //рекурсивно очистить слой
    }
    DeleteIntervals(GetAllIntervals()); //удаляем все интервалы из слоя
    DeleteSublayers(GetSublayers()); //удаляем все подслои
    intervals.clear(); //очистить массив интервалов
    sublayers.clear(); //очистить массив подслоев
    SetTitle(""); //удалить заголовок
    SetDescription(""); //удалить описание
}

void Layer::Serialize(QJsonObject &jsOb)
{
    LayerSerializer::Serialize(jsOb, this);
}

void Layer::Deserialize(QJsonObject &jsOb)
{
    LayerDeserializer::Deserialize(jsOb, this);
}

void Layer::SerializeRecursively(QJsonObject &jsOb)
{
    LayerSerializer::SerializeRecursively(jsOb, this);
}

void Layer::DeserializeRecursively(QJsonObject &jsOb)
{
    LayerDeserializer::DeserializeRecursively(jsOb, this);
}

/*********************************************
GlobalAssistant
**********************************************/

void GlobalAssistant::recursiveDepthCount(idType l)
{
    Layer* layer = GetLayer(l);
    if (!layer) //если такого слоя нет - вернуться
        return;
    if (layer->GetSublayers().empty()) //если нет подслоев - глубина 0, выход
    {
        depthMap[l] = 0;
        return;
    }
    foreach (idType idL, layer->GetSublayers()) { //иначе рекурсивно вычислить глубину каждого подслоя
        recursiveDepthCount(idL);
    }
    DepthCount(l); //и посчитать глубину текущего
}

void GlobalAssistant::DepthCount(idType l)
{
    qint64 max = 0;
    foreach (idType idL, LayersMap[l]->GetSublayers()) { //поиск максимальной глубины у подслоёв
        if (depthMap[idL] > max)
            max = depthMap[idL];
    }
    depthMap[l] = max + 1; //глубина текущего слоя на 1 больше максимальной
}

GlobalAssistant::GlobalAssistant() : QObject()
{

}

void GlobalAssistant::Recount(idType id)
{
    recursiveDepthCount(id);
}

void GlobalAssistant::RecountAll(QVector<idType> Layers)
{
    foreach (idType l, Layers) {
        recursiveDepthCount(l);
    }
}

void GlobalAssistant::ConnectToLayer(Layer *l)
{
    l->SetGlobalAssistant(this);
}

void GlobalAssistant::ConnectToLayers(QVector<Layer *> Layers)
{
    foreach (Layer* l, Layers) {
        ConnectToLayer(l);
    }
}

void GlobalAssistant::AddLayer(Layer *layer)
{
    LayersMap[layer->GetID()] = layer;
}

void GlobalAssistant::AddTimeSpan(TimeSpan *ts)
{
    TimeSpanMap[ts->GetID()] = ts;
}

void GlobalAssistant::DeleteLayer(idType id)
{
    Layer* l = LayersMap[id]; //сохранить указатель на слой
    if(!l) //если нет такого слоя - вернуться
       return;
    foreach (idType ts, l->GetAllIntervals()) { //удалить слой из каждого промежутка
        TimeSpanMap[ts]->DeleteLayer(id);
    }
    LayersMap.remove(id); //удалить по id из кэша
    delete l; //освободить память
}

void GlobalAssistant::DeleteTimeSpan(idType id)
{
    TimeSpan* ts = TimeSpanMap[id]; //сохранить указатель на промежуток
    if(!ts) //если нет такого интервала - вернуться
       return;
    foreach (idType l, ts->GetLayers()) { //удалить промежуток из каждого слоя, в который он входит
        LayersMap[l]->DeleteInterval(id);
    }
    TimeSpanMap.remove(id); //удалить промежуток из кэша по id
    delete ts; //освободить память
}

Layer *GlobalAssistant::GetLayer(idType id)
{
    if (LayersMap.contains(id))
    {
        return LayersMap[id];
    }
    else
    {
        return 0;
    }
}

TimeSpan *GlobalAssistant::GetTimeSpan(idType id)
{
    if (TimeSpanMap.contains(id))
    {
        return TimeSpanMap[id];
    }
    else
    {
        return 0;
    }
}

QVector<Layer *> GlobalAssistant::GetAllLayers()
{
   return LayersMap.values().toVector();
}

QVector<TimeSpan *> GlobalAssistant::GetAllTimeSpans()
{
    return TimeSpanMap.values().toVector();
}

qint64 GlobalAssistant::GetDepth(idType id)
{
    if(depthMap.contains(id))
        return depthMap[id];
    else
        return -1;
}

void GlobalAssistant::SetDepth(idType id, qint64 val)
{
    if(depthMap.contains(id))
        depthMap[id] = val;
}

void GlobalAssistant::Serialize(QJsonObject &jsOb)
{
    GlobalAssistantSerializer::Serialize(jsOb, this);
}

void GlobalAssistant::Deserialize(QJsonObject &jsOb)
{
    GlobalAssistantDeserializer::Deserialize(jsOb, this);
}

GlobalAssistant::~GlobalAssistant()
{
    foreach (idType id, TimeSpanMap.keys()) { //удалить все промежутки
        delete(TimeSpanMap[id]);
    }
    foreach (idType id, LayersMap.keys()) { //удалить все слои
        delete(LayersMap[id]);
    }
}
/*************************************************
*TimeSpanSerializer
* ***********************************************/

void TimeSpanSerializer::Serialize(QJsonObject &jsOb, TimeSpan *ts)
{
    //пишем в jsOb нужные данные
    jsOb["id"] = ts->GetID(); //пишем id
    jsOb["start"] = ts->GetStart().toString(); //пишем время начала в виде строки
    jsOb["end"] = ts->GetEnd().toString(); //то же для конца
    jsOb["title"] = ts->GetTitle();//пишем заголовок
    jsOb["description"] = ts->GetDescription(); //пишем описание

    QJsonArray arrOfLayers; //массив для храниения id связанных слоев
    foreach (idType idL, ts->GetLayers()) { //для каждого id слоя
        QJsonObject idObj; //создать новый объект
        idObj["id"] = idL; //сохранить в нём id слоя
        arrOfLayers.append(idObj); //добавить новый объект к массиву
    }
    jsOb["layers"] = arrOfLayers; //добавить массив к jsOb
}

/*************************************************
*TimeSpanDeserializer
* ***********************************************/
void TimeSpanDeserializer::Deserialize(QJsonObject &jsOb, TimeSpan *ts)
{
    //получаем из jsOb нужные данные

    //не получаем id с помощью toString() на случай, если тип id изменится
    //вместо этого преобразуем в QVariant, который можно привести к любому типу
    if(jsOb.contains("id"))
        ts->SetID(jsOb["id"].toVariant().value<idType>());

    //время начала и конца парсим из строки с помощью ф-ии fromString
    if(jsOb.contains("start"))
        ts->SetStart(QDateTime::fromString(jsOb["start"].toString()));

    if(jsOb.contains("end"))
        ts->SetEnd(QDateTime::fromString(jsOb["end"].toString()));

    //заголовок и описание просто прочитать в виде строки
    if(jsOb.contains("title"))
        ts->SetTitle(jsOb["title"].toString());

    if(jsOb.contains("description"))
        ts->SetDescription(jsOb["description"].toString());

    QVector<idType> layers; //id связанных слоев
    QJsonArray arrOfLayers = jsOb["layers"].toArray(); //массив объектов, в которых хранятся id связанных слоев
    foreach (QJsonValue idObj, arrOfLayers) {
        layers.append(idObj.toObject()["id"].toVariant().value<idType>()); //извлекаем id из каждого объекта
    }
    ts->SetLayers(layers); //устанавливаем слои в ts
}

/*************************************************
*TaskSerializer
* ***********************************************/
void TaskSerializer::Serialize(QJsonObject &jsOb, Task *ts)
{
    ts->TimeSpan::Serialize(jsOb);
    jsOb["isPerformed"] = ts->GetIsPerformed();
}

/*************************************************
*TaskDeserializer
* ***********************************************/

void TaskDeserializer::Deserialize(QJsonObject &jsOb, Task *ts)
{
    ts->TimeSpan::Deserialize(jsOb);
    if(jsOb.contains("isPerformed"))
        ts->SetIsPerformed(jsOb["isPerformed"].toBool());
}

/*************************************************
*EventSerializer
* ***********************************************/

void EventSerializer::Serialize(QJsonObject &jsOb, Event *ts)
{
    ts->TimeSpan::Serialize(jsOb); //сериализуем часть от базового класса

    QJsonArray arrOfTasks; //массив для хранения id задач, присутствующихв событии
    foreach (idType idTsk, ts->GetAllTasks()) {
        QJsonObject idObj; //объект для хранения id задачи
        idObj["id"] = idTsk; //установить поле id во временном объекте
        arrOfTasks.append(idObj); //добавить объект
    }
    jsOb["tasks"] = arrOfTasks;
}

/*************************************************
*EventDeserializer
* ***********************************************/

void EventDeserializer::Deserialize(QJsonObject &jsOb, Event *ts)
{
    ts->Deserialize(jsOb); //десериализуем часть базового класса
    QVector<idType> tasks; //id связанных задач
    QJsonArray arrOfTasks = jsOb["tasks"].toArray(); //массив объектов, в которых хранятся id связанных слоев
    foreach (QJsonValue idObj, arrOfTasks) {
        tasks.append(idObj.toObject()["id"].toVariant().value<idType>()); //извлекаем id из каждого объекта
    }
    ts->SetLayers(tasks); //устанавливаем слои в ts
}

/*************************************************
*GlobalAssistantSerializer
* ***********************************************/

void GlobalAssistantSerializer::Serialize(QJsonObject &jsOb, GlobalAssistant *gs)
{
    QJsonArray TimeSpanArr; //массив хранения объектов, представляющих промежутки
    QJsonArray LayerArr;    //массив для хранения объектов, представляющих слои

    QVector<TimeSpan*> TimeSpans = gs->GetAllTimeSpans(); //массив с указателями на все интервалы
    QVector<Layer*> Layers = gs->GetAllLayers();       //массив с указателями на все слои

    //сериализовать все интервалы
    foreach (TimeSpan* ts, TimeSpans) {
        QJsonObject tsObj; //объект, в который сериализуется интервал
        ts->Serialize(tsObj); //сериализация в объект
        TimeSpanArr.append(tsObj); //добавляем полученный объект к требуемому массиву
    }
    jsOb["TimeSpanArr"] = TimeSpanArr; //добавляем массив с объектами для интервалов в общий объект

    //сериализовать все слои
    foreach (Layer* l, Layers) {
        QJsonObject lObj; //объект, в который сериализуется слой
        l->Serialize(lObj); //сериализация в объект
        lObj["depth"] = gs->GetDepth(l->GetID()); //запись высоты
        LayerArr.append(lObj); //добавляем полученный объект к требуемому массиву
    }
    jsOb["LayerArr"] = LayerArr; //добавляем массив с объектами для слоев в общий объект
}

/*************************************************
*GlobalAssistantDeserializer
* ***********************************************/

void GlobalAssistantDeserializer::Deserialize(QJsonObject &jsOb, GlobalAssistant *gs)
{
    QJsonArray TimeSpanArr = jsOb["TimeSpanArr"].toArray(); //массив хранения объектов, представляющих промежутки
    QJsonArray LayerArr = jsOb["LayerArr"].toArray();       //массив для хранения объектов, представляющих слои

    //десериализовать все интервалы
    for (int i = 0; i < TimeSpanArr.size(); ++i) {
        QJsonObject tsObj = TimeSpanArr[i].toObject();
        TimeSpan* ts = new TimeSpan(); //указатель на новый слой
        ts->Deserialize(tsObj); //десериализация объекта
        ts->SetGlobalAssistant(gs); //устанавливаем ассистента
        gs->AddTimeSpan(ts); //добавляем полученный интервал к ассистенту
    }

    //десериализовать все слои
    for (int i = 0; i < LayerArr.size(); ++i) {
        QJsonObject lObj = LayerArr[i].toObject();
        Layer* l = new Layer(); //указатель на новый слой
        l->Deserialize(lObj); //десериализация слоя
        qint64 depth = lObj["depth"].toDouble(); //получение глубины слоя
        l->SetGlobalAssistant(gs); //устанавливаем ассистента
        gs->AddLayer(l); //добавляем полученный слой к ассистенту
        gs->SetDepth(l->GetID(), depth); //устанавливаем глибину
    }
}

/*************************************************
*LayerSerializer
* ***********************************************/

void LayerSerializer::Serialize(QJsonObject &jsOb, Layer *l)
{
    //сериализуем id, title, description
    jsOb["id"] = l->GetID();
    jsOb["title"] = l->GetTitle();
    jsOb["description"] = l->GetDescription();
    jsOb["idParent"] = l->GetParentLayer();

    QJsonArray arrOfIntervals; //массив для храниния объектов, содержащих id интервалов в слое
    foreach (idType t, l->GetAllIntervals()) { //сериализуем id всех интервалов из слоя
        QJsonObject ot; //новый объект
        ot["id"] = t; //с id текущего интервала
        arrOfIntervals.append(ot); //добавляется в массив
    }
    jsOb["intervals"] = arrOfIntervals; //пишем массив интервалов в общий объект

    QJsonArray arrOfSublayers; //массив для храниния объектов, содержащих id подслоев в слое
    foreach (idType t, l->GetSublayers()) { //сериализуем id всех подслоев из слоя
        QJsonObject ot; //новый объект
        ot["id"] = t; //с id текущего подслоя
        arrOfSublayers.append(ot); //добавляется в массив
    }
    jsOb["sublayers"] = arrOfSublayers; //пишем массив подслоев в общий объект

}

void LayerSerializer::SerializeRecursively(QJsonObject &jsOb, Layer *l)
{
    l->Serialize(jsOb); //обычная сериализация
    QJsonArray arrOfSublayersInf; //массив с информацией о подслоях
    foreach (idType idL, l->GetSublayers()) {
        QJsonObject subObj; //объект для сериализации информации о текущем подслое
        l->GetGlobalAssistant()->GetLayer(idL)->SerializeRecursively(subObj); //получаем подслой из ассистента слоя и сериализуем
                                                                              //этот подслой во временный объект
        arrOfSublayersInf.append(subObj); //добавить информацию о подслое
    }
    jsOb["sublayersInf"] = arrOfSublayersInf; //пишем информацию обо всех подслоях в общий объект
}

/*************************************************
*LayerDeserializer
* ***********************************************/

void LayerDeserializer::Deserialize(QJsonObject &jsOb, Layer *l)
{
    //десериализуем id, title, description
    if(jsOb.contains("id"))
        l->SetID(jsOb["id"].toVariant().value<idType>());

    if(jsOb.contains("title"))
        l->SetTitle(jsOb["title"].toString());

    if(jsOb.contains("description"))
        l->SetDescription(jsOb["description"].toString());

    if(jsOb.contains("idParent"))
        l->SetParentLayer(jsOb["idParent"].toVariant().value<idType>());

    if(jsOb.contains("intervals")) //если есть интервалы
    {
        QJsonArray arrOfIntervals = jsOb["intervals"].toArray(); //получаем массив объектов, содержащих id интервалов
        foreach (QJsonValue tObj, arrOfIntervals) {
            l->AddIntervalStupidly(idType(tObj.toObject()["id"].toString())); //получить id из объекта и добавить к интервалам
        }
    }

    if(jsOb.contains("sublayers")) //если есть подслои
    {
        QJsonArray arrOfSublayers = jsOb["sublayers"].toArray(); //получаем массив объектов, содержащих id слоев
        foreach (QJsonValue tObj, arrOfSublayers) {
            l->AddSublayerStupidly(idType(tObj.toObject()["id"].toString())); //получить id из объекта и добавить к подслоям
        }
    }

}

void LayerDeserializer::DeserializeRecursively(QJsonObject &jsOb, Layer *l)
{
    l->Deserialize(jsOb); //обычная десериализация
    if(jsOb.contains("sublayersInf"))
    {
        QJsonArray arrOfSublayersInf = jsOb["sublayersInf"].toArray(); //массив с информацией о подслоях
        for (int i = 0; i < arrOfSublayersInf.size(); ++i) { //рекурсивно десериализуем каждый подслой
            QJsonObject subL = arrOfSublayersInf[i].toObject();
            Layer* sl = new Layer(); //новый считываемый подслой
            sl->SetGlobalAssistant(l->GetGlobalAssistant()); //устанавливаем подслою ассистента от текущего слоя
            l->GetGlobalAssistant()->AddLayer(sl); //добавляем новый подслой к ассистенту
            sl->DeserializeRecursively(subL); //десериализуем этот новый подслой объектом из массива
        }//foreach
    }//if
}//fnc

/*************************************************
*User
* ***********************************************/

User::User()
{
    gs = new GlobalAssistant();
}

idType User::CreateTimeSpan(QVector<idType> Layers,
                            QDateTime Start,
                            QDateTime End,
                            QString Title,
                            QString Description)
{
    TimeSpan* ts = new TimeSpan(Layers, Start, End, Title, Description, gs);
    return ts->GetID();
}

idType User::CreateLayer(QVector<idType> Intervals, QVector<idType> Sublayers, idType ParentLayer, QString Title, QString Description)
{
    Layer* l = new Layer(Intervals, Sublayers, ParentLayer, Title, Description, gs);
    return l->GetID();
}

User::~User()
{
    delete gs;
}
