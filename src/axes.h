#ifndef AXES_H
#define AXES_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Axes : protected QOpenGLFunctions {
public:

    glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f);
    float radius = 250.0f;
    QOpenGLVertexArrayObject* axis_VAO;
    QOpenGLBuffer* axis_VBO;
    QOpenGLVertexArrayObject* circle_VAO;
    QOpenGLBuffer* circle_VBO;
    unsigned int circle_num_vertices = 100;
    QOpenGLShaderProgram* shader = nullptr;

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

Axes()  {
    initializeOpenGLFunctions();

    // make unit line on x axis
    float axis_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };
    axis_VAO = new QOpenGLVertexArrayObject();
    axis_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    axis_VAO->create();
    axis_VAO->bind();
    axis_VBO->create();
    axis_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    axis_VBO->bind();
    axis_VBO->allocate(axis_vertices, sizeof(float)*2*3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    axis_VAO->release();

    // make unit circle on yz plane
    float dtheta = 2*M_PI/circle_num_vertices;
    float* circle_vertices = new float[circle_num_vertices*3];
    for(unsigned int i=0; i<circle_num_vertices; i++){
        circle_vertices[3*i+0] = 0.0f;
        circle_vertices[3*i+1] = cos(i*dtheta);
        circle_vertices[3*i+2] = sin(i*dtheta);
    }
    circle_VAO = new QOpenGLVertexArrayObject();
    circle_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    circle_VAO->create();
    circle_VAO->bind();
    circle_VBO->create();
    circle_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    circle_VBO->bind();
    circle_VBO->allocate(circle_vertices, sizeof(float)*circle_num_vertices*3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    circle_VAO->release();
    delete[] circle_vertices;

    // make shader
    shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
    shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
    shader->link();
}

// free GPU memory
~Axes(){
    delete circle_VBO;
    delete circle_VAO;
    delete shader;
}

// draw axes
void render(glm::mat4 transform){
    shader->bind();
    glLineWidth(2.0f);
    //transform = glm::translate(transform, location);
    transform = glm::scale(transform, glm::vec3(radius, radius, radius));
    unsigned int transform_id = glGetUniformLocation(shader->programId(), "transform");

    // draw x axis and circle
    shader->setUniformValue("color", 1.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(transform_id, 1, GL_FALSE, glm::value_ptr(transform));
    circle_VAO->bind();
    glDrawArrays(GL_LINE_LOOP, 0, circle_num_vertices);
    circle_VAO->release();
    axis_VAO->bind();
    glDrawArrays(GL_LINES, 0, 2);
    axis_VAO->release();

    // draw y axis and circle
    shader->setUniformValue("color", 0.0f, 1.0f, 0.0f);
    glUniformMatrix4fv(transform_id, 1, GL_FALSE, glm::value_ptr(glm::rotate(transform, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))));
    circle_VAO->bind();
    glDrawArrays(GL_LINE_LOOP, 0, circle_num_vertices);
    circle_VAO->release();
    axis_VAO->bind();
    glDrawArrays(GL_LINES, 0, circle_num_vertices);
    axis_VAO->release();

    // draw z axis and circle
    shader->setUniformValue("color", 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(transform_id, 1, GL_FALSE, glm::value_ptr(glm::rotate(transform, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    circle_VAO->bind();
    glDrawArrays(GL_LINE_LOOP, 0, circle_num_vertices);
    circle_VAO->release();
    axis_VAO->bind();
    glDrawArrays(GL_LINES, 0, circle_num_vertices);
    axis_VAO->release();
}

};

#endif // AXES_H
