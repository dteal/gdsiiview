#ifndef PART_H
#define PART_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <vector>
#include <ctime>
#include "mesh.h"
#include "gdsii.h"

class Part{
public:
bool created = false;
bool initialized = false;
std::string config_filepath;
std::time_t config_write_time;

std::vector<std::shared_ptr<Mesh>>meshes;
std::string filepath = "";
GDSII* gdsii;
glm::mat4 transform = glm::mat4(1.0f);

Part(){}

void initialize(){
    if(!boost::filesystem::exists(filepath) || boost::filesystem::is_directory(filepath)){
        std::cout << "Error: filepath \"" << filepath << "\" not valid" << std::endl;
    }
    config_filepath = filepath;
    config_write_time = boost::filesystem::last_write_time(filepath);

    std::cout << "Initializing part \"" << filepath << "\"" << std::endl;

    gdsii = gdsii_create_gdsii();
    gdsii_read(gdsii, filepath.c_str());

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
}

void render(glm::mat4 transform){
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->render(transform * this->transform);
    }
}

void update(){
    std::time_t current_config_write_time = boost::filesystem::last_write_time(config_filepath);
    if(current_config_write_time > config_write_time){
        deinitialize();
        initialize();
        std::cout << "File \"" << filepath << "\" updated; reloading..." << std::endl;
        config_write_time = current_config_write_time;
        /*for(unsigned int i=0; i<meshes.size(); i++){
            // TODO: re-initialize meshes
        }*/
    }
}

};

#endif
