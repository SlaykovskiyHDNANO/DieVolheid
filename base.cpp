#include "base.h"
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

/*********************************************
idGenerate
**********************************************/
idType idGenerate()
{
    QTime t(0, 0, 0);
    qsrand(t.secsTo(QTime::currentTime()));
    idType id;
    for (int i = 0; i < 10; ++i)
    {
        int tmp = qrand() % 255;
        id.append( (char)tmp );
    }
}

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

void TimeSpan::SetLayers(QVector<Layer *> Layers)
{
    this->layers = Layers;
}



void TimeSpan::AddLayer(Layer *NewLayer)
{
    if(!layers.contains(NewLayer))
        layers.append(NewLayer);
}

void TimeSpan::AddLayers(QVector<Layer *> LayersPtr)
{
    foreach (Layer *l, LayersPtr) {
        AddLayer(l);
    }
}



void TimeSpan::DeleteLayer(Layer *DeletedLayer)
{
    if(layers.contains(DeletedLayer))
        layers.removeOne(DeletedLayer);
}



void TimeSpan::DeleteLayers(QVector<Layer *> LayersPtr)
{
    foreach (Layer *l, LayersPtr) {
        DeleteLayer(l);
    }
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

void Layer::straightSublayersWalk(QVector<Layer *> &watchedLayers, int curDepth)
{
    watchedLayers.append(this);
    if (curDepth <= 0)
        return;
    foreach (Layer* l, sublayers) {
        l->straightSublayersWalk(watchedLayers, curDepth - 1);
    }
}

void Layer::reverseSublayersWalk(QVector<Layer *> &watchedLayers, int curDepth)
{
    if (curDepth <= 0)
        watchedLayers.append(this);
        return;
    foreach (Layer* l, sublayers) {
        l->straightSublayersWalk(watchedLayers, curDepth - 1);
    }
    watchedLayers.append(this);
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
        l->SetParentLayer(this);
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
        l->SetParentLayer(this);
    }
}

Layer *Layer::GetRoot(QVector<Layer *> &trace)
{
    Layer* tmp = this;
    while(tmp->ParentLayer)
    {
        trace.append(tmp);
        tmp = tmp->ParentLayer;
    }
    return tmp;
}

Layer::Layer() : ParentLayer(0)
{
    assistant = new LayerAssistant(this);
}

Layer::Layer(idType Id, QVector<TimeSpan *> Intervals, QVector<Layer *> Sublayers, Layer *parentLayer, QString Title, QString Description):
    intervals(Intervals), sublayers(Sublayers)
{
    SetID(Id);
    SetParentLayer(parentLayer);
    SetTitle(Title);
    SetDescription(Description);
    assistant = new LayerAssistant(this);
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
    //QVector<Layer *> watcherLayers;
    //straightSublayersWalk(watcherLayers, MaxDepth);
    return assistant->GetAllLayers();
}

QVector<TimeSpan *> Layer::GetAllIntervals()
{
    return assistant->GetAllTimeSpans();
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

void Layer::SetParentLayer(idType Id)
{
    this->ParentLayer = assistant->GetLayer(Id);
}

void Layer::AddInterval(idType Id)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    TimeSpan* curSpn = root->assistant->GetTimeSpan(Id);
    if (curSpn)
    {
        if(!this->intervals.contains(curSpn))
        {
            foreach (Layer* layer, trace) {
                layer->assistant->AddTimeSpan(curSpn);
            }//foreach
            root->assistant->AddTimeSpan(curSpn);
            intervals.append(curSpn);
            curSpn->AddLayer(this);
        }
    }//if
}//fnc

void Layer::AddInterval(TimeSpan *Interval)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    bool tmpEl = false;
    if(!root->assistant->GetTimeSpan(Interval->GetID()))
    {
        root->assistant->AddTimeSpan(Interval);
        tmpEl = true;
    }
    AddInterval(Interval->GetID());
    if(tmpEl)
    {
        root->assistant->DeleteTimeSpan(Interval->GetID());
    }
}

void Layer::NewInterval(QDateTime Start, QDateTime End, QString IntervalTitle, QString IntervalDescription)
{
    TimeSpan *pts = new TimeSpan(idGenerate(), QVector<Layer*>(0), Start, End, IntervalTitle, IntervalDescription);
    AddInterval(pts);
}

void Layer::AddIntervals(QVector<idType> IntervalsId)
{
    foreach (idType id, IntervalsId) {
        AddInterval(id);
    }
}

void Layer::AddIntervals(QVector<TimeSpan *> IntervalsPtr)
{
    foreach (TimeSpan* pts, IntervalsPtr) {
        AddInterval(pts);
    }
}

void Layer::AddSublayer(idType Id)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    Layer* curLayer = root->assistant->GetLayer(Id);
    if (curLayer)
    {
        if(!this->sublayers.contains(curLayer))
        {
            sublayers.append(curLayer);
            curLayer->SetParentLayer(this);

            QVector<Layer*> recLrs = curLayer->GetSublayers(curLayer->assistant->GetMaxDepth());
            QVector<TimeSpan*> recTs = curLayer->GetAllIntervals();
            trace.append(root);
            trace.removeOne(curLayer);
            foreach (Layer* layer, trace) {
                foreach (Layer* LayerToAdd, recLrs) {
                    layer->assistant->AddLayer(LayerToAdd);
                    layer->assistant->DepthCount(layer);
                }
                foreach (TimeSpan* TsToAdd, recTs) {
                    layer->assistant->AddTimeSpan(TsToAdd);
                }
            }//foreach


        }//if
    }//if
}

void Layer::AddSublayer(Layer *Sublayer)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    bool tmpLr = false;
    if(!root->assistant->GetLayer(Sublayer->GetID()))
    {
        tmpLr = true;
        root->assistant->AddLayer(Sublayer);
    }
    AddSublayer(Sublayer->GetID());
    if(tmpLr)
    {
        root->assistant->DeleteLayer(Sublayer->GetID());
    }
}

void Layer::NewSublayer(idType Id, QString SublayerTitle, QString SublayerDescription)
{
    Layer* l = new Layer(Id, QVector<TimeSpan*>(0), QVector<Layer*>(0), 0, SublayerTitle, SublayerDescription);
    AddSublayer(l);
}

void Layer::AddSublayers(QVector<idType> SublayersId)
{
    foreach (idType id, SublayersId) {
        AddSublayer(id);
    }
}

void Layer::AddSublayers(QVector<Layer *> SublayersPtr)
{
    foreach (Layer* pL, SublayersPtr) {
        AddSublayer(pL);
    }
}

void Layer::DeleteInterval(idType Id)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    TimeSpan* curSpn = root->assistant->GetTimeSpan(Id);
    if (curSpn)
    {
        if(this->intervals.contains(curSpn))
        {
            foreach (Layer* layer, trace) {
                layer->assistant->DeleteTimeSpan(curSpn->GetID());
            }//foreach
            intervals.removeOne(curSpn);
            curSpn->DeleteLayer(this);
            if(curSpn->GetLayers().isEmpty())
                delete curSpn;
        }
    }//if
}

void Layer::DeleteInterval(TimeSpan *Interval)
{
    DeleteInterval(Interval->GetID());
}

void Layer::DeleteIntervals(QVector<idType> IntervalsId)
{
    foreach (idType i,IntervalsId) {
        DeleteInterval(i);
    }
}

void Layer::DeleteIntervals(QVector<TimeSpan *> IntervalsPtr)
{
    foreach (TimeSpan* ts, IntervalsPtr) {
        DeleteInterval(ts);
    }
}

void Layer::DeleteSublayer(idType Id)
{
    QVector<Layer*> trace;
    Layer* root = GetRoot(trace);
    Layer* curLayer = root->assistant->GetLayer(Id);
    if (curLayer)
    {
        if(this->sublayers.contains(curLayer))
        {
            sublayers.removeOne(curLayer);

            QVector<Layer*> recLrs = curLayer->GetSublayers(curLayer->assistant->GetMaxDepth());
            QVector<TimeSpan*> recTs = curLayer->GetAllIntervals();
            trace.append(root);
            trace.removeOne(curLayer);
            foreach (Layer* layer, trace) {
                foreach (Layer* LayerToDelete, recLrs) {
                    layer->assistant->DeleteLayer(LayerToDelete->GetID());
                    layer->assistant->DepthCount(layer);
                }
                foreach (TimeSpan* TsToDelete, recTs) {
                    layer->assistant->DeleteTimeSpan(TsToDelete->GetID());
                }
            }//foreach

            curLayer->Clear();

        }//if
    }//if
}

void Layer::DeleteSublayer(Layer *Sublayer)
{
    DeleteSublayer(Sublayer->GetID());
}

void Layer::DeleteSublayers(QVector<idType> SublayersId)
{
    foreach (idType i, SublayersId) {
        DeleteSublayer(i);
    }
}

void Layer::DeleteSublayers(QVector<Layer *> SublayersPtr)
{
    foreach (Layer* l, SublayersPtr) {
        DeleteSublayer(l);
    }
}

void Layer::Clear()
{
    foreach (Layer* l, sublayers) {
        l->Clear();
    }
    delete assistant;
    delete this;
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
    assistant->Recount();
    assistant->Repick();
}

/*********************************************
LayerAssistant
**********************************************/

void LayerAssistant::recursiveDataGet(Layer *layer)
{
    recursiveLayersMap.clear();
    recursiveTimeSpanMap.clear();
    AddLayer(layer);
    for (int i = 0; i < layer->intervals.size(); ++i)
        AddTimeSpan(layer->intervals[i]);
    foreach (Layer* l, layer->sublayers) {
        recursiveDataGet(l);
    }
}

void LayerAssistant::recursiveDepthCount(Layer *layer)
{
    if (!layer)
        return;
    if (layer->sublayers.empty())
    {
        layer->assistant->maxDepth = 0;
        return;
    }
    foreach (Layer* l, layer->sublayers) {
        recursiveDepthCount(l);
    }
    layer->assistant->DepthCount(layer);
}

void LayerAssistant::DepthCount(Layer *layer)
{
    qint64 max = 0;
    foreach (Layer* l, node->sublayers) {
        if (l->assistant->maxDepth > max)
            max = l->assistant->maxDepth;
    }
    layer->assistant->maxDepth = max + 1;
}

LayerAssistant::LayerAssistant(Layer *Node)
{
    node = Node;
    //recursiveDataGet(node);
    //recursiveDepthCount(node);
}

void LayerAssistant::Recount()
{
    recursiveDepthCount(node);
}

void LayerAssistant::Repick()
{
    recursiveDataGet(node);
}

qint64 LayerAssistant::GetMaxDepth()
{
    return maxDepth;
}

void LayerAssistant::AddLayer(Layer *layer)
{
    recursiveLayersMap.insert(layer->GetID(), layer);
}

void LayerAssistant::AddTimeSpan(TimeSpan *ts)
{
    recursiveTimeSpanMap.insert(ts->GetID(), ts);
}

void LayerAssistant::DeleteLayer(idType id)
{
    QMultiMap <idType, Layer*>::iterator it = recursiveLayersMap.find(id);
    recursiveLayersMap.erase(it);
}

void LayerAssistant::DeleteTimeSpan(idType id)
{
    QMultiMap <idType, TimeSpan*>::iterator it = recursiveTimeSpanMap.find(id);
    recursiveTimeSpanMap.erase(it);
}

Layer *LayerAssistant::GetLayer(idType id)
{
    if (recursiveLayersMap.contains(id))
    {
        return recursiveLayersMap.value(id);
    }
    else
    {
        return 0;
    }
}

TimeSpan *LayerAssistant::GetTimeSpan(idType id)
{
    if (recursiveTimeSpanMap.contains(id))
    {
        return recursiveTimeSpanMap.value(id);
    }
    else
    {
        return 0;
    }
}

QVector<Layer *> LayerAssistant::GetAllLayers()
{
   return recursiveLayersMap.values().toSet().toList().toVector(); //без повторений
}

QVector<TimeSpan *> LayerAssistant::GetAllTimeSpans()
{
    return recursiveTimeSpanMap.values().toSet().toList().toVector(); //без повторений
}

/*void LayerAssistant::Serialize(QJsonObject &jsOb)
{
    QJsonArray arrOfIntervals;
    QJsonArray arrOfLayers;
    foreach (idType t, recursiveTimeSpanMap.keys()) {
        arrOfIntervals.append(t);
    }
    foreach (idType t, recursiveLayersMap.keys()) {
        arrOfLayers.append(t);
    }
    jsOb["maxDepth"] = maxDepth;
    jsOb["intervals"] = arrOfIntervals;
    jsOb["layers"] = arrOfLayers;
}

void LayerAssistant::Deserialize(QJsonObject &jsOb)
{
    lightLayerDeserialization(jsOb);
    if(!jsOb.contains("intervals") || !jsOb.contains("layers") || !jsOb.contains("maxDepth"))
        return;
    QJsonArray arrOfIntervals = jsOb["intervals"].toArray();
    for (int i = 0; i < arrOfIntervals.size(); ++i)
    {
        QJsonObject ot = arrOfIntervals[i].toObject();
        TimeSpan* t = new TimeSpan();
        t->Deserialize(ot);
        intervals.append(t);
    }
    QJsonArray arrOfIntervals = jsOb["intervals"].toArray();
    for (int i = 0; i < arrOfIntervals.size(); ++i)
    {
        QJsonObject ot = arrOfIntervals[i].toObject();
        TimeSpan* t = new TimeSpan();
        t->Deserialize(ot);
        intervals.append(t);
    }
}*/

