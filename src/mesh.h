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
    layout (location = 1) in vec3 nor;                  \n\
    uniform mat4 transform;                             \n\
    out vec3 normal;                                    \n\
    void main(){                                        \n\
        gl_Position = transform * vec4(pos.xyz, 1.0f);  \n\
        normal = nor;                                   \n\
    }";
const char* fragment_source = "                         \n\
    #version 330 core                                   \n\
    uniform vec3 color;                                 \n\
    in vec3 normal;                                     \n\
    out vec4 FragColor;                                 \n\
    void main(){                                        \n\
        vec3 light1 = vec3(0.70, 0.42, 0.58);           \n\
        float diff1 = max(dot(light1, normal), 0.0);    \n\
        vec3 light2 = vec3(-0.70, -0.23, -0.58);        \n\
        float diff2 = max(dot(light2, normal), 0.0);    \n\
        vec3 final = (diff1+diff2*0.5) * color;         \n\
        FragColor = vec4(final.xyz, 1.0f);              \n\
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

                // loop through points of polygon

                // determine CW vs CCW edges by integrating
                GDSII_POINT* point = element->point;
                float area = 0.0f;
                while(point->next != NULL){ // skip last point, which is a duplicate of the first
                    area += (point->next->x-point->x)*(point->next->y+point->y); // 2 * area between line and x-axis
                    point = point->next;
                }
                // clockwise if area is positive; CCW otherwise

                point = element->point;
                while(point->next != NULL){ // skip last point, which is a duplicate of the first
                    // assume CCW
                    float scale = 1000.0f;
                    glm::vec2 p1 = glm::vec2(point->x/scale, point->y/scale);
                    glm::vec2 p2 = glm::vec2(point->next->x/scale, point->next->y/scale);
                    if(area > 0){ glm::vec2 temp = p1; p1 = p2; p2 = temp; }

                    glm::vec2 n = glm::vec2(p2.y-p1.y, p1.x-p2.x);
                    n /= glm::length(n);
                    float z1 = zbounds[0]; float z2 = zbounds[1];
                    float delta = 0.1/scale; // shift all vertices inward by less than a database unit
                    p1 -= n*delta;
                    p2 -= n*delta;
                    float tris[] = {
                        p1.x, p1.y, z1, n.x, n.y, 0,
                        p2.x, p2.y, z1, n.x, n.y, 0,
                        p2.x, p2.y, z2, n.x, n.y, 0,
                        p2.x, p2.y, z2, n.x, n.y, 0,
                        p1.x, p1.y, z2, n.x, n.y, 0,
                        p1.x, p1.y, z1, n.x, n.y, 0,
                    };
                    for(unsigned int j=0; j<6*6; j++){
                        vertices.push_back(tris[j]);
                    }
                    point = point->next;
                }

                // end polygon
            }
            element = element->next;
        }
        structure = structure->next;
    }

    float* data = new float[vertices.size()];
    for(unsigned int i=0; i<vertices.size(); i++){
        data[i] = vertices[i];
    }
    num_vertices = vertices.size()/6;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    shader = std::shared_ptr<Shader>(new Shader(vertex_source, fragment_source));
    initialized = true;
    delete[] data;
}

void deinitialize(){
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    initialized = false;
}

// free GPU memory
~Mesh(){
    if(initialized){
        deinitialize();
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
