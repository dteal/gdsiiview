#ifndef AXES_H
#define AXES_H

#include <QOpenGLFunctions>             // OpenGL functions
#include <QOpenGLVertexArrayObject>     // OpenGL vertex array
#include <QOpenGLBuffer>                // OpenGL vertex buffer
#include <QOpenGLShaderProgram>         // OpenGL shader
#include <QSharedPointer>               // for safer pointer handling
#include "glm/glm.hpp"                  // GLM matrices
#include "glm/gtc/type_ptr.hpp"         // GLM matrices to OpenGL data

class Axes : protected QOpenGLFunctions {
public:

    QSharedPointer<QOpenGLVertexArrayObject> axis_VAO;
    QSharedPointer<QOpenGLBuffer> axis_VBO;
    QSharedPointer<QOpenGLVertexArrayObject> circle_VAO;
    QSharedPointer<QOpenGLBuffer> circle_VBO;
    QSharedPointer<QOpenGLShaderProgram> shader;
    unsigned int circle_num_vertices = 100;

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

    // make a unit line on the x axis
    GLfloat axis_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };
    axis_VAO = QSharedPointer<QOpenGLVertexArrayObject>(new QOpenGLVertexArrayObject());
    axis_VBO = QSharedPointer<QOpenGLBuffer>(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    axis_VAO->create();
    axis_VAO->bind();
    axis_VBO->create();
    axis_VBO->bind();
    axis_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    axis_VBO->allocate(axis_vertices, sizeof(GLfloat)*2*3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    axis_VAO->release();

    // make a unit circle on the yz plane
    float circle_dtheta = 2*M_PI/circle_num_vertices;
    GLfloat* circle_vertices = new GLfloat[circle_num_vertices*3];
    for(unsigned int i=0; i<circle_num_vertices; i++){
        circle_vertices[3*i+0] = 0.0f;
        circle_vertices[3*i+1] = cos(i*circle_dtheta);
        circle_vertices[3*i+2] = sin(i*circle_dtheta);
    }
    circle_VAO = QSharedPointer<QOpenGLVertexArrayObject>(new QOpenGLVertexArrayObject());
    circle_VBO = QSharedPointer<QOpenGLBuffer>(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    circle_VAO->create();
    circle_VAO->bind();
    circle_VBO->create();
    circle_VBO->bind();
    circle_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    circle_VBO->allocate(circle_vertices, sizeof(GLfloat)*circle_num_vertices*3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    circle_VAO->release();
    delete[] circle_vertices;

    // make shader
    shader = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram());
    shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source);
    shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source);
    shader->link();
}

~Axes(){
    // assume there is an active OpenGL context
    axis_VAO->release();
    axis_VBO->release();
    circle_VAO->release();
    circle_VBO->release();
    shader->release();
}

void render(glm::mat4 transform){
    shader->bind();
    glLineWidth(2.0f);

    float circle_radius = 0.4f; // fill 0.4*2 = 80% of canvas height
    transform = glm::scale(transform, glm::vec3(circle_radius, circle_radius, circle_radius));
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
    glDrawArrays(GL_LINES, 0, 2);
    axis_VAO->release();

    // draw z axis and circle
    shader->setUniformValue("color", 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(transform_id, 1, GL_FALSE, glm::value_ptr(glm::rotate(transform, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
    circle_VAO->bind();
    glDrawArrays(GL_LINE_LOOP, 0, circle_num_vertices);
    circle_VAO->release();
    axis_VAO->bind();
    glDrawArrays(GL_LINES, 0, 2);
    axis_VAO->release();

    shader->release();
}

};

#endif // AXES_H
