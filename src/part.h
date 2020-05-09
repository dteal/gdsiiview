#ifndef PART_H
#define PART_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <memory>
#include <vector>
#include <ctime>

#include "glm/glm.hpp"
#include "mesh.h"
#include "gdsii.h"

class Part : public QObject{
public:
bool created = false;
bool initialized = false;

QString filepath = "";
QString stlfilepath = "";
QFileSystemWatcher* watcher;

std::vector<std::shared_ptr<Mesh>>meshes;
GDSII* gdsii;
glm::mat4 transform = glm::mat4(1.0f);
Part(){
    watcher = new QFileSystemWatcher();
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &Part::update_file);
}

void initialize(){
    qDebug() << "open part: " << filepath;
    if(filepath == ""){ qDebug() << "invalid part filepath"; return; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){ qDebug() << "invalid part filepath"; return; }
    watcher->addPath(filepath);
    watcher->files().removeDuplicates();

    qDebug() << "Initializing part \"" << filepath << "\"";

    gdsii = gdsii_create_gdsii();
    gdsii_read(gdsii, filepath.toStdString().c_str());

    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->gdsii = gdsii;
        meshes[i]->initialize();
    }

    initialized = true;
}

void deinitialize(){
    gdsii_delete_gdsii(gdsii);
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->gdsii = gdsii;
        meshes[i]->deinitialize();
    }

    initialized = true;
}

~Part(){
    if(initialized){
        deinitialize();
    }
    delete watcher;
}

void render(glm::mat4 transform){
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->render(transform * this->transform);
    }
}

void update_file(){
    deinitialize();
    initialize();
    //std::cout << "File \"" << filepath << "\" updated; reloading..." << std::endl;
    /*for(unsigned int i=0; i<meshes.size(); i++){
        // TODO: re-initialize meshes
    }*/
}

};

#endif
