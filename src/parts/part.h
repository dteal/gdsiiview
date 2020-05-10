#ifndef PART_H
#define PART_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <memory>
#include <vector>
#include <ctime>
#include "glm/glm.hpp"
#include "gdsiimesh.h"
#include "gdsii.h"

class Part : public QObject{
public:
    bool created = false; // makes parsing config file simpler
    bool initialized = false; // tracks whether

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
    if((filepath == "") || !(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){
        qDebug() << "Error: part has invalid part filepath: " << filepath;
        return;
    }
    watcher->addPath(filepath);
    watcher->files().removeDuplicates();

    gdsii = gdsii_create_gdsii();
    gdsii_read(gdsii, filepath.toStdString().c_str());
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->gdsii = gdsii;
        meshes[i]->initialize();
    }
    initialized = true;
}

void deinitialize(){
    initialized = false;
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->deinitialize();
    }
    gdsii_delete_gdsii(gdsii);
}

~Part(){
    if(initialized){
        deinitialize();
    }
    delete watcher;
}

void render(glm::mat4 transform){
    if(initialized){
        for(unsigned int i=0; i<meshes.size(); i++){
            meshes[i]->render(transform * this->transform);
        }
    }
}

void update_file(){
    deinitialize();
    initialize();
}

};

#endif
