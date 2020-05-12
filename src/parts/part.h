#ifndef PART_H
#define PART_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <memory>
#include <vector>
#include <ctime>
#include <limits>
#include "glm/glm.hpp"
#include "mesh.h"
#include "gdsii.h"
#include "image.h"

class Part : public QObject{
public:

    enum part_type{
        PART_GDSII,
        PART_IMAGE,
    };

    part_type type;
    bool initialized = false;
    bool created = false;

    QString filepath = "";
    QString stlfilepath = "";
    QFileSystemWatcher* watcher;

    std::vector<std::shared_ptr<Mesh>>meshes;
    std::shared_ptr<Image> image;
    GDSII* gdsii;

    glm::mat4 transform = glm::mat4(1.0f);
    glm::mat4 rotate = glm::mat4(1.0f); // to help with normal rendering

Part(){
    /*
    watcher = new QFileSystemWatcher();
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &Part::update_file);
    */
}

void initialize(){
    if((filepath == "") || !(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){
        qDebug() << "Error: part filepath invalid: " << filepath;
        return;
    }
    //watcher->addPath(filepath);
    //watcher->files().removeDuplicates();

    if(type==PART_GDSII){
        gdsii = gdsii_create_gdsii();
        gdsii_read(gdsii, filepath.toStdString().c_str());
        for(unsigned int i=0; i<meshes.size(); i++){
            meshes[i]->gdsii = gdsii;
            meshes[i]->initialize();
        }
    }else if(type==PART_IMAGE){
        image->initialize(filepath);
    }

    initialized = true;
}

void deinitialize(){
    initialized = false;
    if(type==PART_GDSII){
        for(unsigned int i=0; i<meshes.size(); i++){
            meshes[i]->deinitialize();
        }
        gdsii_delete_gdsii(gdsii);
    }else if(type==PART_IMAGE){
        image->deinitialize();
    }
}

~Part(){
    if(initialized){
        deinitialize();
    }
    //delete watcher;
}

void render(glm::mat4 transform, glm::mat4 rotate){
    if(!initialized){ return; }
    if(type==PART_GDSII){
        for(unsigned int i=0; i<meshes.size(); i++){
            meshes[i]->render(transform * this->transform, rotate*this->rotate);
        }
    }else if(type==PART_IMAGE){
        image->render(transform * this->transform, rotate*this->rotate);
    }
}

glm::vec4 get_bounds(glm::mat4 transform){
    glm::vec4 bounds = glm::vec4(std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest());
    for(unsigned int i=0; i<meshes.size(); i++){
        glm::vec4 meshbounds = meshes[i]->get_bounds(transform * this->transform);
        if(meshbounds[0] < bounds[0]) bounds[0] = meshbounds[0];
        if(meshbounds[1] > bounds[1]) bounds[1] = meshbounds[1];
        if(meshbounds[2] < bounds[2]) bounds[2] = meshbounds[2];
        if(meshbounds[3] > bounds[3]) bounds[3] = meshbounds[3];
    }
    return bounds;
}

void update_file(QString filepath){
    if(filepath == this->filepath){ // this function is called when any watched file is changed; make sure it's the right one
        deinitialize();
        initialize();
    }
}

};

#endif
