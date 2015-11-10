#include "base.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

/*********************************************
Timespan
**********************************************/

void TimeSpan::SetID(idType Id)
{
    this->id = Id;
}

TimeSpan::TimeSpan()
{
    SetID("");
}

TimeSpan::TimeSpan(idType Id, QVector<Layer *> Layers, QDateTime Start, QDateTime End, QString Title, QString Description)
{
    SetID(Id);
    this->layers = Layers;
    SetStart(Start);
    SetEnd(End);
    SetTitle(Title);
    SetDescription(Description);
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

QVector<Layer *> TimeSpan::GetLayers()
{
    return this->layers;
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

void TimeSpan::AddLayer(idType Id)
{

}

void TimeSpan::AddLayer(Layer *NewLayer)
{

}

void TimeSpan::AddLayers(QVector<idType> LayersId)
{

}

void TimeSpan::AddLayers(QVector<Layer *> LayersPtr)
{

}

void TimeSpan::DeleteLayer(idType Id)
{

}

void TimeSpan::DeleteLayer(Layer *DeletedLayer)
{

}

void TimeSpan::DeleteLayers(QVector<idType> LayersId)
{

}

void TimeSpan::DeleteLayers(QVector<Layer *> LayersPtr)
{

}

void TimeSpan::Serialize(QJsonObject& jsOb)
{
    jsOb["id"] = GetID();
    jsOb["start"] = GetStart().toString();
    jsOb["end"] = GetEnd().toString();
    jsOb["title"] = GetTitle();
    jsOb["description"] = GetDescription();
}

void TimeSpan::Deserialize(QJsonObject& jsOb)
{
    if(jsOb.contains("id"))
        SetID(jsOb["id"].toVariant().value<idType>());

    if(jsOb.contains("start"))
        SetStart(QDateTime::fromString(jsOb["start"].toString()));

    if(jsOb.contains("end"))
        SetEnd(QDateTime::fromString(jsOb["end"].toString()));

    if(jsOb.contains("title"))
        SetTitle(jsOb["title"].toString());

    if(jsOb.contains("description"))
        SetDescription(jsOb["description"].toString());
}

/***************************************************/

/**************************************************
Task
**************************************************/

Task::Task() : TimeSpan()
{
    SetIsPerformed(0);
}

Task::Task(idType Id, QVector<Layer *> Layers, QDateTime Start, QDateTime End, QString Title, QString Description, bool IsPerformed):
    TimeSpan(Id, Layers, Start, End, Title, Description)
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
    this->TimeSpan::Serialize(jsOb);
    //TimeSpan::Serialize(jsOb);
    jsOb["isPerformed"] = GetIsPerformed();
}

void Task::Deserialize(QJsonObject &jsOb)
{
    this->TimeSpan::Deserialize(jsOb);
    if(jsOb.contains("isPerformed"))
        SetIsPerformed(jsOb["isPerformed"].toBool());
}
/***************************************************/

/**************************************************
Event
**************************************************/

void Event::Serialize(QJsonObject &jsOb)
{

}

void Event::Deserialize(QJsonObject &jsOb)
{

}

/*********************************************
Layer
**********************************************/

void Layer::wideSublayersWalk(QVector<Layer *> &watchedLayers, int curDepth)
{
    watchedLayers.append(this);
    if (curDepth <= 0)
        return;
    foreach (Layer* l, sublayers) {
        l->wideSublayersWalk(watchedLayers, curDepth - 1);
    }
}

void Layer::lightLayerSerialization(QJsonObject &jsOb)
{
    jsOb["id"] = GetID();
    jsOb["title"] = GetTitle();
    jsOb["description"] = GetDescription();
}

void Layer::lightLayerDeserialization(QJsonObject &jsOb)
{
    if(jsOb.contains("id"))
        SetID(jsOb["id"].toVariant().value<idType>());

    if(jsOb.contains("title"))
        SetTitle(jsOb["title"].toString());

    if(jsOb.contains("description"))
        SetDescription(jsOb["description"].toString());
}

void Layer::heavyLayerSerialization(QJsonObject &jsOb)
{
    lightLayerSerialization(jsOb);
    QJsonArray arrOfIntervals;
    foreach (TimeSpan* t, intervals) {
        QJsonObject ot;
        t->Serialize(ot);
        arrOfIntervals.append(ot);
    }
    jsOb["intervals"] = arrOfIntervals;
}

void Layer::heavyLayerDeserialization(QJsonObject &jsOb)
{
    lightLayerDeserialization(jsOb);
    if(!jsOb.contains("intervals"))
        return;
    QJsonArray arrOfIntervals = jsOb["intervals"].toArray();
    for (int i = 0; i < arrOfIntervals.size(); ++i)
    {
        QJsonObject ot = arrOfIntervals[i].toObject();
        TimeSpan* t = new TimeSpan();
        t->Deserialize(ot);
        intervals.append(t);
    }
}

void Layer::lightRecursiveLayerSerialization(QJsonObject &jsOb, int curDepth)
{
    lightLayerSerialization(jsOb);
    if (curDepth <= 0)
        return;
    QJsonArray arrOfSublayers;
    foreach (Layer* l, sublayers) {
        QJsonObject ob;
        l->lightRecursiveLayerSerialization(ob, curDepth - 1);
        arrOfSublayers.append(ob);
    }
    jsOb["sublayers"] = arrOfSublayers;
}

void Layer::lightRecursiveLayerDeserialization(QJsonObject &jsOb, int curDepth)
{
    lightLayerDeserialization(jsOb);
    if (curDepth <= 0)
        return;
    if(!jsOb.contains("sublayers"))
        return;
    QJsonArray arrOfSublayers = jsOb["sublayers"].toArray();
    for (int i = 0; i < arrOfSublayers.size(); ++i)
    {
        QJsonObject ob = arrOfSublayers[i].toObject();
        Layer *l = new Layer();
        l->lightRecursiveLayerDeserialization(ob, curDepth - 1);
        sublayers.append(l);
    }
}

void Layer::heavyRecursiveLayerSerialization(QJsonObject &jsOb, int curDepth)
{
    heavyLayerSerialization(jsOb);
    if (curDepth <= 0)
        return;
    QJsonArray arrOfSublayers;
    foreach (Layer* l, sublayers) {
        QJsonObject ob;
        l->heavyRecursiveLayerSerialization(ob, curDepth - 1);
        arrOfSublayers.append(ob);
    }
    jsOb["sublayers"] = arrOfSublayers;
}

void Layer::heavyRecursiveLayerDeserialization(QJsonObject &jsOb, int curDepth)
{
    heavyLayerDeserialization(jsOb);
    if (curDepth <= 0)
        return;
    if(!jsOb.contains("sublayers"))
        return;
    QJsonArray arrOfSublayers = jsOb["sublayers"].toArray();
    for (int i = 0; i < arrOfSublayers.size(); ++i)
    {
        QJsonObject ob = arrOfSublayers[i].toObject();
        Layer *l = new Layer();
        l->heavyRecursiveLayerDeserialization(ob, curDepth - 1);
        sublayers.append(l);
    }
}

Layer::Layer()
{

}

Layer::Layer(idType Id, QVector<TimeSpan *> Intervals, QVector<Layer *> Sublayers, Layer *parentLayer, QString Title, QString Description):
    intervals(Intervals), sublayers(Sublayers)
{
    SetID(Id);
    SetParentLayer(parentLayer);
    SetTitle(Title);
    SetDescription(Description);
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

Layer *Layer::GetParrentLayer()
{
    return this->ParentLayer;
}

QVector<Layer *> Layer::GetSublayers(int MaxDepth)
{
    QVector<Layer *> watcherLayers;
    wideSublayersWalk(watcherLayers, MaxDepth);
    return watcherLayers;
}

QVector<TimeSpan *> Layer::GetAllIntervals()
{

}

void Layer::SetID(idType id)
{
    this->id = id;
}

void Layer::SetTitle(QString NewTitle)
{
    this->title = NewTitle;
}

void Layer::SetDescription(QString NewDescription)
{
    this->description = NewDescription;
}

void Layer::SetParentLayer(Layer *NewParent)
{
    this->ParentLayer = NewParent;
}

void Layer::AddInterval(TimeSpan *Interval)
{
    intervals.append(Interval);
}

void Layer::AddSublayer(Layer *Sublayer)
{
    Sublayer->SetParentLayer(this);
    sublayers.append(Sublayer);
}

void Layer::NewSublayer(idType Id, QString SublayerTitle, QString SublayerDescription)
{
    Layer* l = new Layer(Id, QVector<TimeSpan*>(0), QVector<Layer*>(0), this, SublayerTitle, SublayerDescription);
    sublayers.append(l);
}

void Layer::Serialize(QJsonObject &jsOb, int depth, Layer::typeOfSerialization ts)
{
    if (ts == Layer::LIGHT_SERIALIZATION)
        lightRecursiveLayerSerialization(jsOb, depth);
    if (ts == Layer::HEAVY_SERIALIZATION)
        heavyRecursiveLayerSerialization(jsOb, depth);
}

void Layer::Deserialize(QJsonObject &jsOb, int depth, Layer::typeOfDeserialization td)
{
    if (td == Layer::LIGHT_DESERIALIZATION)
        lightRecursiveLayerDeserialization(jsOb, depth);
    if (td == Layer::HEAVY_DESERIALIZATION)
        heavyRecursiveLayerDeserialization(jsOb, depth);
}
