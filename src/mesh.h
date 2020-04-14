#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "shader.h"
#include "gdsii.h"

class Mesh{
private:
GLuint VAO;
GLuint VBO;
std::shared_ptr<Shader>shader;

// this is messy, but easier than separate files
const char* vertex_source = "                           \n\
    #version 330 core                                   \n\
    layout (location = 0) in vec3 pos;                  \n\
    uniform mat4 transform;                             \n\
    void main(){                                        \n\
        gl_Position = transform * vec4(pos.xyz, 1.0f);  \n\
    }";
const char* fragment_source = "                         \n\
    #version 330 core                                   \n\
    uniform vec3 color;                                 \n\
    out vec4 FragColor;                                 \n\
    void main(){                                        \n\
        FragColor = vec4(color.xyz, 1.0f);              \n\
    }";

public:
bool created = false;
bool initialized = false;
glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec2 zbounds = glm::vec2(-1.0f, 1.0f);
int gdslayer = 1;
bool export_stl = false;
std::string stlfilepath = "";
GDSII* gdsii;
unsigned int num_vertices = 0;

void initialize(){
    std::cout << "Mesh initialized with layer " << gdslayer << std::endl;
    if(export_stl){
        std::cout << "Exporting mesh to stl at " << stlfilepath << std::endl;
    }

    std::vector<float> vertices;

    GDSII_STRUCTURE* structure = gdsii->structure;
    while(structure != NULL){
        GDSII_ELEMENT* element = structure->element;
        while(element != NULL){
            if(element->layer == gdslayer && element->type == ELEMENT_TYPE_BOUNDARY){
                GDSII_POINT* point = element->point;
                while(point != NULL){
                    if(point->next == NULL){
                        break;
                    }
                    float scale = 1000.0f;
                    float x1 = point->x/scale; float y1 = point->y/scale;
                    float x2 = point->next->x/scale; float y2 = point->next->y/scale;
                    if(abs(x1)+abs(x2)+abs(y1)+abs(y2)>500000){
                        std::cout << "ERROR" << std::endl;
                    }else{
                        float z1 = zbounds[0]; float z2 = zbounds[1];
                        float tris[] = {
                            x1, y1, z1,
                            x1, y1, z2,
                            x2, y2, z1,
                            x2, y2, z2,
                            x2, y2, z1,
                            x1, y1, z2,
                        };
                        for(unsigned int j=0; j<6*3; j++){
                            vertices.push_back(tris[j]);
                        }
                    }
                    point = point->next;
                }
            }
            element = element->next;
        }
        structure = structure->next;
    }

    float* data = new float[vertices.size()];
    for(unsigned int i=0; i<vertices.size(); i++){
        data[i] = vertices[i];
    }
    num_vertices = vertices.size()/3;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    shader = std::shared_ptr<Shader>(new Shader(vertex_source, fragment_source));
    initialized = true;
    delete[] data;
}

// free GPU memory
~Mesh(){
    if(initialized){
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
}

// draw mesh
void render(glm::mat4 view){
    if(initialized){
        shader->use();
        shader->set_mat4("transform", view);
        shader->set_vec3("color", color);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
}

};

#endif
