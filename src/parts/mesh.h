#ifndef MESH_H
#define MESH_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <limits>
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
    std::vector<glm::vec3>mesh_points;
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
    uniform mat4 rotate;                                \n\
    out vec4 normal;                                    \n\
    void main(){                                        \n\
        gl_Position = transform * vec4(pos.xyz, 1.0f);  \n\
        normal = rotate * vec4(nor.xyz, 1.0f);          \n\
    }";
const char* fragment_source = "                         \n\
    #version 330 core                                   \n\
    uniform vec3 color;                                 \n\
    in vec4 normal;                                     \n\
    out vec4 FragColor;                                 \n\
    void main(){                                        \n\
        vec3 light1 = vec3(-0.70, 0.42, 0.58);          \n\
        float diff1 = max(dot(light1, normal.xyz), 0.0);\n\
        vec3 light2 = vec3(0.70, -0.42, -0.58);         \n\
        float diff2 = max(dot(light2, normal.xyz), 0.0);\n\
        float ambient = 0.1;                            \n\
        vec3 final = (ambient+2*(diff1+diff2*0.5))*color;\n\
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

    if(zbounds.y > zbounds.x){ // ensure z bound order
        zbounds = glm::vec2(zbounds.y, zbounds.x);
    };

    GDSII_STRUCTURE* structure = gdsii->structure;
    while(structure != NULL){
        if(QString(structure->name) == "$$$CONTEXT_INFO$$$"){
            // skip KLayout PCELL structures
            structure = structure->next;
            continue;
        }
        GDSII_ELEMENT* element = structure->element;
        while(element != NULL){
            if(element->layer == gdslayer && element->type == ELEMENT_TYPE_BOUNDARY){

                // Only consider polygons with at least 3 points.
                if(element->point == NULL ||
                   element->point->next == NULL ||
                   element->point->next->next == NULL){continue;}

                float scale = 1000.0f; // (GDSII database units per model unit) TODO: update to use GDSII file units

                // Loop through the points of the polygon in two passes.
                // During the first pass, count the number of points, extract
                // the points and calculate normal vectors to each edge, and
                // determine whether the edge winds clockwise (CW) or
                // counterclockwise (CCW).
                unsigned int num_points = 0;
                std::vector<glm::vec2> points;
                std::vector<glm::vec2> normals;
                float area = 0.0f;
                GDSII_POINT* point = element->point;
                while(point->next != NULL){ // skip last point, which is a duplicate of the first
                    num_points += 1;
                    glm::vec2 scaled_point = glm::vec2(point->x/scale, point->y/scale);
                    glm::vec2 scaled_point_next = glm::vec2(point->next->x/scale, point->next->y/scale);
                    glm::vec2 normal = glm::vec2(scaled_point_next.y-scaled_point.y,
                                                 scaled_point.x-scaled_point_next.x);
                    normal /= glm::length(normal);
                    points.push_back(scaled_point);
                    normals.push_back(normal);
                    area += (point->next->x-point->x)*(point->next->y+point->y); // 2 * area between line and x-axis
                    point = point->next;
                }
                bool CW = area > 0; // polygon is clockwise if area is positive, CCW otherwise

                // During the second pass, (1) offset each point along its
                // adjacent edge normals by a small amount delta to help
                // triangulate weird polygons, (2) create edge polygons,
                // and (3) prepare to triangulate the polygon.
                struct triangulateio in, out;
                in.numberofpoints = num_points;
                in.numberofpointattributes = 0;
                in.pointmarkerlist = NULL;
                in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
                in.numberofsegments = num_points;
                in.segmentmarkerlist = NULL;
                in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
                in.numberofholes = 0;
                in.holelist = NULL;
                //in.numberofholes = num_points;
                //in.holelist = (REAL *) malloc(in.numberofsegments * 2 * sizeof(REAL));
                for(unsigned int i=0; i<num_points; i++){

                    float delta = 0.01f;
                    //float delta = 0.1f/scale; // fix to database units

                    // get points and normals
                    int di = num_points+1;
                    if(CW){ di = num_points-1; };
                    glm::vec2 p1 = points[i];
                    glm::vec2 p2 = points[(i+di)%num_points];
                    glm::vec2 normal_1a = normals[(i)%num_points];
                    glm::vec2 normal_1b = normals[(i+di)%num_points];
                    glm::vec2 normal_2a = normals[(i+di)%num_points];
                    glm::vec2 normal_2b = normals[(i+di+di)%num_points];
                    if(CW){
                        normal_1a = -normal_1a;
                        normal_1b = -normal_1b;
                        normal_2a = -normal_2a;
                        normal_2b = -normal_2b;
                    }
                    p1 -= delta*(normal_1a + normal_1b);
                    p2 -= delta*(normal_2a + normal_2b);
                    float z1 = zbounds[0]; float z2 = zbounds[1];

                    float tris[] = {
                        p1.x, p1.y, z1, normal_1b.x, normal_1b.y, 0,
                        p2.x, p2.y, z1, normal_1b.x, normal_1b.y, 0,
                        p2.x, p2.y, z2, normal_1b.x, normal_1b.y, 0,
                        p2.x, p2.y, z2, normal_1b.x, normal_1b.y, 0,
                        p1.x, p1.y, z2, normal_1b.x, normal_1b.y, 0,
                        p1.x, p1.y, z1, normal_1b.x, normal_1b.y, 0,
                    };
                    for(unsigned int j=0; j<6*6; j++){
                        vertices.push_back(tris[j]);
                    }

                    in.pointlist[i*2] = p1.x;
                    in.pointlist[i*2+1] = p1.y;
                    in.segmentlist[i*2] = i;
                    in.segmentlist[i*2+1] = (i+1) % num_points;

                    /*
                    glm::vec2 hole_marker = p1 + p2;
                    hole_marker *= 0.5;
                    hole_marker += 0.5f*delta*normal_1b;
                    in.holelist[i*2] = hole_marker.x;
                    in.holelist[i*2+1] = hole_marker.y;
                    */
                }

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

                // store points to help fit view later
                for(unsigned int i=0; i<points.size(); i++){
                    mesh_points.push_back(glm::vec3(points[i].x, points[i].y, z1));
                    mesh_points.push_back(glm::vec3(points[i].x, points[i].y, z2));
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
void render(glm::mat4 view, glm::mat4 rotate){
    if(initialized){
        // TODO: rotate normals
        shader->bind();
        unsigned int matlocation = glGetUniformLocation(shader->programId(), "transform");
        glUniformMatrix4fv(matlocation, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int rotlocation = glGetUniformLocation(shader->programId(), "rotate");
        glUniformMatrix4fv(rotlocation, 1, GL_FALSE, glm::value_ptr(rotate));
        unsigned int collocation = glGetUniformLocation(shader->programId(), "color");
        glUniform3fv(collocation, 1, glm::value_ptr(color));
        VAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        VAO->release();
        glm::vec4 test = glm::vec4(1.0f,0.0f,0.0f, 1.0f);
        test = rotate*test;
    }
}

glm::vec4 get_bounds(glm::mat4 transform){
    glm::vec4 bounds = glm::vec4(std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest());
    for(unsigned int i=0; i<mesh_points.size(); i++){
        glm::vec4 pos = transform*glm::vec4(mesh_points[i], 1.0f);
        if(pos[0] < bounds[0]) bounds[0] = pos[0]; // xmin
        if(pos[0] > bounds[1]) bounds[1] = pos[0]; // xmax
        if(pos[1] < bounds[2]) bounds[2] = pos[1]; // ymin
        if(pos[1] > bounds[3]) bounds[3] = pos[1]; // ymax
    }
    return bounds;
}

};

#endif
