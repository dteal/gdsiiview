#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <sstream>
#include <ctime>
#include <memory>
#include "part.h"
#include "mesh.h"

glm::vec2 cursor_position = glm::vec2(0.0f, 0.0f);
glm::vec2 screen_size = glm::vec2(0.0f, 0.0f);

glm::vec3 camera_position = glm::vec3(-1000.0f, 0.0f, 0.0f);
glm::vec3 camera_direction = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);
/*
 * pan: move camera perpendicular to camera_direction (x/y)
 * zoom: move camera along camera_direction
 * rotate: rotate camera around center of screen?
 */

bool orbiting = false;
float theta = 0.0f; // camera angle, degrees
float phi = M_PI/2.0f; // camera angle, degrees
float zoom = 0.001f;
std::string config_filepath;
std::time_t config_write_time;

std::vector<std::shared_ptr<Part>>parts;

void glfw_error_callback(int error, const char* description){
    std::cout << "Error in GLFW: " << error << ":" << description << std::endl;
}

void window_size_callback(GLFWwindow*, int width, int height){
    glViewport(0, 0, width, height);
    screen_size = glm::vec2(width, height);
}

void cursor_movement_callback(GLFWwindow*, double x, double y){
    if(orbiting){
        theta += (x-cursor_position.x)*1.0f;
        phi += (y-cursor_position.y)*1.0f;
    }
    cursor_position = glm::vec2(x, y);
}

void cursor_button_callback(GLFWwindow*, int button, int action, int){
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        orbiting = true;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        orbiting = false;
    }
}

void cursor_scroll_callback(GLFWwindow*, double, double y){
    if(y>0){
        zoom *= 1.05;
    }
    if(y<0){
        zoom /= 1.05;
    }
}

GLFWwindow* initialize_window(std::string name){
    GLFWwindow* window;
    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()){
        std::cout << "Error initializing GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(500, 500, name.c_str(), NULL, NULL);
    if(!window){
        glfwTerminate();
        std::cout << "Error creating GLFW window" << std::endl;
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    GLenum err = glewInit();
    if(GLEW_OK != err){
        std::cout << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return nullptr;
    }
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, cursor_movement_callback);
    glfwSetMouseButtonCallback(window, cursor_button_callback);
    glfwSetScrollCallback(window, cursor_scroll_callback);
    glEnable(GL_DEPTH_TEST);
    return window;
}

void terminate_window(GLFWwindow* window){
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool parse_configuration(std::string filepath){
    if(!boost::filesystem::exists(filepath) || boost::filesystem::is_directory(filepath)){
        std::cout << "Error: filepath \"" << filepath << "\" not valid" << std::endl; return false;
    }
    config_filepath = filepath;
    config_write_time = boost::filesystem::last_write_time(filepath);
    boost::filesystem::path relativepath(filepath);
    relativepath = relativepath.parent_path();
    std::ifstream infile(filepath);
    std::string line;
    std::shared_ptr<Part>temppart = std::shared_ptr<Part>(new Part());
    std::shared_ptr<Mesh>tempmesh = std::shared_ptr<Mesh>(new Mesh());
    int linenumber = 0;
    // parse configuration file line by line
    while(std::getline(infile, line)){
        // break each line into whitespace-separated words
        linenumber += 1;
        line = line + " "; // add whitespace to flush last word in line...
        std::vector<std::string>commands;
        std::stringstream ss;
        bool quoted = false;
        for(unsigned int i=0; i<line.size(); i++){
            switch(line[i]){
                case '"': quoted = !quoted; break;
                case '\n':
                case '\r':
                case ' ':
                case '\t': // ...here.
                    if(!quoted){
                        if(ss.str().size()>0){
                            commands.push_back(ss.str());
                            ss.str("");
                        }
                    }else{ ss << line[i]; } // quoted strings keep whitespace
                    break;
                default:
                    ss << line[i];
            }
        }
        // apply commands
        if(commands[0] == "gdsii:"){
            if(temppart->created){
                if(tempmesh->created){ temppart->meshes.push_back(tempmesh);
                                       tempmesh = std::shared_ptr<Mesh>(new Mesh()); }
                parts.push_back(temppart);
            }
            temppart = std::shared_ptr<Part>(new Part());
            temppart->filepath = (relativepath / commands[1]).c_str();
            temppart->created = true;
        }else if(commands[0] == "comment:"){
        }else if(commands[0] == "transform:"){
        }else if(commands[0] == "layer:"){
            if(tempmesh->created){ temppart->meshes.push_back(tempmesh); }
            tempmesh = std::shared_ptr<Mesh>(new Mesh());
            tempmesh->gdslayer = std::stoi(commands[1]);
            tempmesh->created = true;
        }else if(commands[0] == "rotate:"){
            glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f);
            switch(commands[1][0]){
                case 'x': axis = glm::vec3(1.0f, 0.0f, 0.0f); break;
                case 'y': axis = glm::vec3(0.0f, 1.0f, 0.0f); break;
                case 'z': axis = glm::vec3(0.0f, 0.0f, 1.0f); break;
                default: std::cout << "Error: unknown axis in configuration file in line " << linenumber << std::endl;
            }
            temppart->transform = glm::rotate(glm::mat4(1.0f), glm::radians(std::stof(commands[2])), axis)*temppart->transform;
        }else if(commands[0] == "translate:"){
            temppart->transform = glm::translate(glm::mat4(1.0f), glm::vec3(std::stof(commands[1]), std::stof(commands[2]), std::stof(commands[3]))) * temppart->transform;
        }else if(commands[0] == "color:"){
            tempmesh->color = glm::vec3(std::stoi(commands[1])/255.0f, std::stoi(commands[2])/255.0f, std::stoi(commands[3])/255.0f);
        }else if(commands[0] == "zbounds:"){
            tempmesh->zbounds = glm::vec2(std::stof(commands[1]), std::stof(commands[2]));
        }else if(commands[0] == "stl:"){
            tempmesh->export_stl = true;
            tempmesh->stlfilepath = (relativepath / commands[1]).c_str();
        }else{
            std::cout << "Error: could not parse configuration file line " << linenumber << std::endl;
        }
    }
    // store last remaining part and/or mesh
    if(temppart->created){
        if(tempmesh->created){ temppart->meshes.push_back(tempmesh); }
        parts.push_back(temppart);
    }
    return true;
}

int main(int argc, char* argv[]){
    if(argc < 2 || !parse_configuration(argv[1])){
        std::cout << "Usage: gdsiiview example.gdsiiview" << std::endl;
        return 0;
    }

    GLFWwindow* window = initialize_window(argv[1]);
    if(window==nullptr) return 1;

    // initialize all parts/meshes (e.g., open files, tesselate...)
    // after OpenGL context is available
    for(unsigned int i=0; i<parts.size(); i++){
        parts[i]->initialize();
    }

    while(!glfwWindowShouldClose(window)){
        // watch configuration file for updates
        std::time_t current_config_write_time = boost::filesystem::last_write_time(config_filepath);
        if(current_config_write_time > config_write_time){
            std::cout << "Configuration file updated; reloading..." << std::endl;
            config_write_time = current_config_write_time;
            parts.clear();
            parse_configuration(config_filepath);
            for(unsigned int i=0; i<parts.size(); i++){
                parts[i]->initialize();
            }
        }else{
            for(unsigned int i=0; i<parts.size(); i++){
                parts[i]->update();
            }
        }

        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //transform XYZ device space to screen XY
        glm::mat4 view;
        view = glm::ortho(-0.5f*screen_size.x/screen_size.y, 0.5f*screen_size.x/screen_size.y, -0.5f, 0.5f, -100.0f, 100.0f);
        view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // camera rotate and zoom
        view = glm::scale(view, glm::vec3(zoom, zoom, zoom));
        view = glm::rotate(view, glm::radians(phi), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));

        for(unsigned int i=0; i<parts.size(); i++){
            parts[i]->render(view);
        }

        glfwSwapBuffers(window);
    }

    terminate_window(window);

    return 0;
}
