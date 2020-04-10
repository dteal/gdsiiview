#ifndef PART_H
#define PART_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <vector>
#include <ctime>
#include "mesh.h"

class Part{
public:
bool created = false;
bool initialized = false;
std::string config_filepath;
std::time_t config_write_time;

std::vector<std::shared_ptr<Mesh>>meshes;
std::string filepath = "";
glm::mat4 transform = glm::mat4(1.0f);

Part(){}

~Part(){
    if(initialized){
    }
}

void initialize(){
    if(!boost::filesystem::exists(filepath) || boost::filesystem::is_directory(filepath)){
        std::cout << "Error: filepath \"" << filepath << "\" not valid" << std::endl;
    }
    config_filepath = filepath;
    config_write_time = boost::filesystem::last_write_time(filepath);

    std::cout << "Initializing part \"" << filepath << "\"" << std::endl;

    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->initialize();
    }

    initialized = true;
}

void render(glm::mat4 transform){
    for(unsigned int i=0; i<meshes.size(); i++){
        meshes[i]->render(transform * this->transform);
    }
}

void update(){
    std::time_t current_config_write_time = boost::filesystem::last_write_time(config_filepath);
    if(current_config_write_time > config_write_time){
        // re-initialize this part
        std::cout << "File \"" << filepath << "\" updated; reloading..." << std::endl;
        config_write_time = current_config_write_time;
        for(unsigned int i=0; i<meshes.size(); i++){
            // re-initialize meshes
        }
    }
}

};

#endif
