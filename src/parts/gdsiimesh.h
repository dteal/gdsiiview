#ifndef MESH_H
#define MESH_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "gdsii.h"

extern "C" {
    #define ANSI_DECLARATORS
    typedef double REAL;
    typedef void VOID; // required on linux (maybe not Windows?)
    #include "triangle.h"
}

class Mesh : protected QOpenGLFunctions {
public:
    bool created = false;
    bool initialized = false;
    glm::vec3 color = glm::vec3(1.0f, 0.5f, 1.0f);
    glm::vec2 zbounds = glm::vec2(-1.0f, 1.0f);
    int gdslayer = 1;
    bool export_stl = false;
    std::string stlfilepath = "";
    GDSII* gdsii;
    unsigned int num_vertices = 0;
    QOpenGLVertexArrayObject* VAO;
    QOpenGLBuffer* VBO;
    QOpenGLShaderProgram* shader = nullptr;

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
        vec3 final = 2*(diff1+diff2*0.5) * color;       \n\
        FragColor = vec4(final.xyz, 1.0f);              \n\
    }";

Mesh()  {
    initializeOpenGLFunctions();
}

void initialize(){
    //std::cout << "Mesh initialized with layer " << gdslayer << std::endl;
    if(export_stl){
        //std::cout << "Exporting mesh to stl at " << stlfilepath << std::endl;
    }

    std::vector<float> vertices;

    GDSII_STRUCTURE* structure = gdsii->structure;
    while(structure != NULL){
        GDSII_ELEMENT* element = structure->element;
        while(element != NULL){
            if(element->layer == gdslayer && element->type == ELEMENT_TYPE_BOUNDARY){
                // TODO: ensure z2>z1 or whichever way it is so that normals are correct

                // loop through points of polygon

                // determine CW vs CCW edges by integrating
                GDSII_POINT* point = element->point;
                float area = 0.0f;
                while(point->next != NULL){ // skip last point, which is a duplicate of the first
                    area += (point->next->x-point->x)*(point->next->y+point->y); // 2 * area between line and x-axis
                    point = point->next;
                }
                // clockwise if area is positive; CCW otherwise

                float scale = 1000.0f;
                point = element->point;
                int num_points = 0;
                while(point->next != NULL){ // skip last point, which is a duplicate of the first
                    num_points += 1;
                    // assume CCW
                    glm::vec2 p1 = glm::vec2(point->x/scale, point->y/scale);
                    glm::vec2 p2 = glm::vec2(point->next->x/scale, point->next->y/scale);
                    if(area > 0){ glm::vec2 temp = p1; p1 = p2; p2 = temp; }

                    glm::vec2 n = glm::vec2(p2.y-p1.y, p1.x-p2.x);
                    n /= glm::length(n);
                    float z1 = zbounds[0]; float z2 = zbounds[1];
                    //float delta = 0.1/scale; // shift all vertices inward by less than a database unit
                    //p1 -= n*delta;
                    //p2 -= n*delta;
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

                struct triangulateio in, out;
                in.numberofpoints = num_points;
                in.numberofpointattributes = 0;
                in.pointmarkerlist = NULL;
                in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
                in.numberofsegments = num_points;
                in.segmentmarkerlist = NULL;
                in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
                point = element->point;
                //while(point->next != NULL){ // skip last point, which is a duplicate of the first
                for(int i=0; i<num_points; i++){
                    in.pointlist[i*2] = point->x/scale;
                    in.pointlist[i*2+1] = point->y/scale;
                    in.segmentlist[i*2] = i;
                    in.segmentlist[i*2+1] = (i+1) % num_points;
                    point = point->next;
                }
                in.numberofholes = 0;
                in.holelist = NULL;
                in.numberofregions = 0;
                in.regionlist = NULL;
                // need set of vertices, segments
                // eventually, see which triangles border edge, on which side, etc...
                // -p = planar straight line graph
                // -z = number from zero
                // -V = verbose
                // -Q = quiet
                out.pointlist = NULL;
                out.pointmarkerlist = NULL;
                out.trianglelist = NULL;
                out.segmentlist = NULL;
                out.segmentmarkerlist = NULL;
                triangulate((char*)"pzQ", &in, &out, NULL);
                float z1 = zbounds[0]; float z2 = zbounds[1];
                int num_corners = out.numberofcorners;
                for(int i=0; i<out.numberoftriangles; i++){
                    float tris[] = {
                        // TODO: move points to account for GDS hole problems
                        // TODO: make sure normals are right direction (z2>z1)
                        (float)out.pointlist[out.trianglelist[i*num_corners]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners]*2+1], z1,0,0,1,
                        (float)out.pointlist[out.trianglelist[i*num_corners+1]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners+1]*2+1], z1,0,0,1,
                        (float)out.pointlist[out.trianglelist[i*num_corners+2]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners+2]*2+1], z1,0,0,1,
                        (float)out.pointlist[out.trianglelist[i*num_corners]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners]*2+1], z2,0,0,-1,
                        (float)out.pointlist[out.trianglelist[i*num_corners+1]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners+1]*2+1], z2,0,0,-1,
                        (float)out.pointlist[out.trianglelist[i*num_corners+2]*2+0],
                        (float)out.pointlist[out.trianglelist[i*num_corners+2]*2+1], z2,0,0,-1,
                    };

                    for(unsigned int j=0; j<6*6; j++){
                        vertices.push_back(tris[j]);
                    }
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

    VAO = new QOpenGLVertexArrayObject();
    VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    VAO->create();
    VAO->bind();
    VBO->create();
    VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    VBO->bind();
    VBO->allocate(data, sizeof(float)*vertices.size());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    VAO->release();

    if(shader == nullptr){
        // shader won't compile vertex shader twice for some reason, so only do it once
        // TODO: figure out why
        shader = new QOpenGLShaderProgram();
        shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
        shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
        shader->link();
    }

    delete[] data;
    initialized = true;
}

void deinitialize(){
    initialized = false;
    delete VBO;
    delete VAO;
}

// free GPU memory
~Mesh(){
    if(initialized){
        deinitialize();
    }
    delete shader;
}

// draw mesh
void render(glm::mat4 view){
    if(initialized){
        shader->bind();
        unsigned int matlocation = glGetUniformLocation(shader->programId(), "transform");
        glUniformMatrix4fv(matlocation, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int collocation = glGetUniformLocation(shader->programId(), "color");
        glUniform3fv(collocation, 1, glm::value_ptr(color));
        VAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        VAO->release();
    }
}

};

#endif
