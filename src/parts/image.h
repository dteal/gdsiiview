#ifndef IMAGE_H
#define IMAGE_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Image : protected QOpenGLFunctions {
public:
    bool created = false;
    bool initialized = false;
    glm::vec3 color = glm::vec3(1.0f, 0.5f, 1.0f);
    glm::vec2 zbounds = glm::vec2(-1.0f, 1.0f);
    int gdslayer = 1;
    bool export_stl = false;
    std::string stlfilepath = "";
    unsigned int num_vertices = 0;
    QOpenGLVertexArrayObject* VAO;
    QOpenGLBuffer* VBO;
    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLTexture* texture;

// this is messy, but easier than separate files
const char* vertex_source = "                           \n\
    #version 330 core                                   \n\
    layout (location = 0) in vec3 pos;                  \n\
    layout (location = 1) in vec3 nor;                  \n\
    uniform mat4 transform;                             \n\
    out vec3 normal;                                    \n\
    void main(){                                        \n\
        gl_Position = transform * vec4(pos.xyz, 1.0f);  \n\
        normal = nor;//normalize(transform * vec4(nor.xyz, 1.0f)).xyz;                                   \n\
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

Image()  {
    initializeOpenGLFunctions();
}

void initialize(){
    // position, normal, texture
    float image_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };

    // position, normal
    float box_vertices[] = {
        // top
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        // bottom
        0.0f, 0.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        1.0f, 0.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        1.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        1.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        0.0f, 1.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f, 0.0f, 0.0f,-1.0f,
        // left
        0.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,-1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,-1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,-1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,
        // right
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // back
        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        // front
        0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    };

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

    initialized = true;
}

void deinitialize(){
    initialized = false;
    delete VBO;
    delete VAO;
}

// free GPU memory
~Image(){
    if(initialized){
        deinitialize();
    }
    delete shader;
}

// draw mesh
void render(glm::mat4 view){
    if(initialized){
        // TODO: rotate normals
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

#endif // IMAGE_H
