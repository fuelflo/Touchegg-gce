/**
 *This work is licensed under the
 *Creative Commons Attribuzione 3.0
 *Unported License. To view a copy
 *of this license, visit
 *http://creativecommons.org/licenses/by/3.0/
 *
 *Please note that this is not Touchégg nor Touchégg-gui,
 *which author is José Expósito <jose.exposito89@gmail.com>.
 *This is a gui interface to edit
 *Touchégg configuration file alternative to Touchégg-gui.
 *
 *@author Raffaele Pertile <raffarti@yahoo.it>
 */
#include "memory.h"

QStringList* Memory::groupsList;
QStringList* Memory::propsList;
QStringList* Memory::appsList;
QHash<QString, App*>* Memory::appsHash;
QHash<QString, Group*>* Memory::groupsHash;
type_propsHash* Memory::propsHash;
int Memory::groupCounter;

//Reminder: all memory variables and mothonds are STATIC
//Note: Lists are used to keep elements ordered for users, Hash->keys() would be random
Memory::Memory(QObject *parent) : QObject(parent)
{
    groupsList = new QStringList();
    propsList = new QStringList();
    appsList = new QStringList();
    appsHash = new QHash<QString, App*>();
    groupsHash = new QHash<QString, Group*>();
    propsHash = new QHash<QString, QString>();
    groupCounter = 0;
}

//---Memory -- GROUP


QString Memory::addGroup(QString apps){
    QString name;
    if (apps == "All")
        name = tr("All");
    else
        name = QString(tr("Group ")).append(QString::number(++groupCounter));
    groupsList->append(name);
    groupsHash->insert(name,new Group(name));
    int k = 0;
    //extracting applications from apps string
    while (true){
        QString part = apps.section(',',k,k,QString::SectionSkipEmpty);
        if (part.isEmpty()) break; //due to SectionSkipEmpty, only possible if string is done
        while (part.startsWith(" ")) part.remove(0,1); //those blanc spaces are worthless
        while (part.endsWith(" ")) part.chop(1);
        if (!part.isEmpty()) //se qualcosa � rimasto...
            addApp(part, name);
        k++;
    }
    return name;
}

Group* Memory::getGroup(QString name){
    return groupsHash->value(name);
}

QStringList* Memory::getGroupsNames(){
    return groupsList;
}

QList<Group*> Memory::getGroups(){
    return groupsHash->values();
}

void Memory::removeGroup(QString name){
    groupsList->removeAll(name);
    groupsHash->remove(name);
    foreach(App* a, *appsHash) //destroy group applications
        if (!a->getGroup()->getName().compare(name))
            removeApp(a->getName());
}


//---Memory -- PROPERTY

void Memory::addProp(QString name, QString value){
    if(propsList->contains(name)) return;
    propsList->append(name);
    propsHash->insert(name,value);
}

QString Memory::getProp(QString name){
    return propsHash->value(name);
}

QStringList* Memory::getProps(){
    return propsList;
}

void Memory::removeProp(QString name){
    propsList->removeAll(name);
    propsHash->remove(name);
}


//---Memory -- APPLICATION


void Memory::addApp(QString name, QString group){
    if(appsList->contains(name)) return;
    if (!groupsList->contains(group)){
        groupsList->append(group);
        groupsHash->insert(group,new Group(group));
    }
    appsList->append(name);
    appsHash->insert(name,new App(name, getGroup(group)));
}

void Memory::addApp(QString name){
    if(appsList->contains(name)) return;
    addGroup(name);
    appsList->append(name);
    appsHash->insert(name,new App(getGroup(name)->getName()));
}

App* Memory::getApp(QString name){
    return appsHash->value(name);
}

QStringList* Memory::getAppsNames(){
    return appsList;
}

QList<App*> Memory::getApps(){
    return appsHash->values();
}

void Memory::removeApp(QString name){
    appsList->removeAll(name);
    appsHash->remove(name);
}


//--- GROUP



Group::Group(QString name){
    this->name = name;
    gestsList = new QStringList();
    gestsHash = new QHash<QString, Gesture*>();
}

QString Group::getName(){
    return name;
}


//--- GROUP -- App

void Group::addApp(QString name){
    Memory::addApp(name, this->name);
}

void Group::addApps(QString names){
    int k = 0;
    while (true){//extracting apps names from names string
        QString part = names.section(',',k,k,QString::SectionSkipEmpty);
        if (part.isEmpty()) break;
        while (part.startsWith(" ")) part.remove(0,1);
        while (part.endsWith(" ")) part.chop(1);
        if (!part.isEmpty())
            addApp(part);
        k++;
    }
}

App* Group::getApp(QString name){
    App *ret = Memory::getApp(name);
    return ret->getGroup() == this?ret:NULL;//this get app is specific for this group, not to memory, filtering
}

QStringList Group::getAppsNames(){
    QStringList *ret = new QStringList();
    foreach(App *a, Memory::getApps()){//group have no reference to its apps, so is necessary to costruct one.
        if (a->getGroup()->getName() == name)
            ret->append(a->getName());
    }
    return *ret;
}

QList<App*> Group::getApps(){
    QList<App*> *ret = new QList<App*>();
    foreach(App *a, Memory::getApps()){//group have no reference to its apps, so is necessary to costruct one.
        if (a->getGroup() == this)
            ret->append(a);
    }
    return *ret;
}


//--- GROUP -- Gesture

void Group::addGest(int fingers, Lists::GestureType type, Lists::GestureDirection direction){
    Gesture *g = new Gesture(fingers, type, direction, this);
    if(gestsList->contains(*g)){
        qDebug("Aptempting to insert an existing gesture, skipping.");
        return;
    }
    gestsList->append(g->toString());
    gestsHash->insert(g->toString(), g);
}

void Group::addGest(Gesture *gesture){
    if(gestsList->contains(*gesture))return;
    gesture->setGroup(this);
    gestsList->append(*gesture);
    gestsHash->insert(*gesture, gesture);
}

Gesture* Group::getGest(int fingers, Lists::GestureType type, Lists::GestureDirection direction){
    Gesture *g = new Gesture(fingers, type, direction, this);
    if(!gestsList->contains(*g))return NULL;
    return gestsHash->value(*g);
}

QList<Gesture*> Group::getGests(){
    return gestsHash->values();
}

QList<Gesture*> Group::getSortedGestures(){
    gestsList->sort();
    QList<Gesture*> ret;
    foreach (QString ges, *gestsList) {
        ret.append(gestsHash->value(ges));
    }return ret;
}

void Group::removeGesture(int fingers, Lists::GestureType type, Lists::GestureDirection direction){
    Gesture *g = new Gesture(fingers, type, direction, this);
    gestsList->removeAll(*g);
    gestsHash->remove(*g);
}

void Group::removeGesture(Gesture gest){
    gestsList->removeAll(gest);
    gestsHash->remove(gest);
}


//--- Application




App::App(QString name, Group *group){
    this->group = group;
    this->name = name;
}

App::App(QString name, QString group){
    Group *g = Memory::getGroup(group);
    if (g == NULL) return;
    this->group = g;
    this->name = name;
}

App::App(QString name){
    this->group = new Group(name);
    this->name = name;
}

QString App::getName(){
    return name;
}

Group* App::getGroup(){
    return group;
}

void App::changeGroup(QString name){
    Group *g = Memory::getGroup(name);
    if (g == NULL) return;
    group = g;
}

void App::changeGroup(Group *group){
    this->group = group;
}


//--- Gesture



Gesture::Gesture(int fingers, Lists::GestureType type, Lists::GestureDirection direction, Group *parent){
    this->fingers = fingers;
    this->type = type;
    this->direction = direction;
    this->parent = parent;
}

Gesture::Gesture(Gesture *ges){

    fingers = ges->fingers;
    type = ges->type;
    direction = ges->direction;
    parent = ges->parent;
    setAction(new Action(ges->act));
}

void Gesture::destroy(){
    parent->removeGesture(fingers, type, direction);
}

void Gesture::setGroup(Group *group){
    this->parent = group;
}

int Gesture::getFingers(){
    return fingers;
}

Lists::GestureType Gesture::getType(){
    return type;
}

Lists::GestureDirection Gesture::getDirection(){
    return direction;
}

void Gesture::setAction(Lists::ActionType type){
    act = new Action(type);
}

void Gesture::setAction(Action *act){
    this->act = act;
}

Action* Gesture::getAction(){
    return act;
}

void Gesture::removeAction(){
    act = NULL;
}

QString Gesture::toString(){

    return QString::number(fingers)
            .append("/").append(Lists::toString(type))
            .append("/").append(Lists::toString(direction));
}

//--- Action


Action::Action(Lists::ActionType type){
    this->type = type;
    paramsList = new QStringList();
    parHash = new QHash<QString,QString>();
}

Action::Action(Action *act){
    type = act->type;
    paramsList = new QStringList(*act->paramsList);
    parHash = new QHash<QString,QString>(*act->parHash);
}

void Action::addParam(QString name){
    if (paramsList->contains(name))return;
    paramsList->append(name);
}

void Action::addParam(QString name, QString value){
    if (paramsList->contains(name))return;
    paramsList->append(name);
    parHash->insert(name, value);
}

Lists::ActionType Action::getType(){
    return type;
}

QStringList Action::getParamKeys(){
    return *paramsList;
}

QString Action::getParamValue(QString key){
    return parHash->value(key);
}

void Action::removeParam(QString name){
    paramsList->removeAll(name);
    parHash->remove(name);
}

void Action::changeParamValue(QString name, QString value){
    parHash->remove(name);
    parHash->insert(name, value);
}
