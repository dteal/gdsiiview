#ifndef IMAGE_H
#define IMAGE_H

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMessageBox>
#include <QFileInfo>
#include <QImage>

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Image : protected QOpenGLFunctions {
public:
    QString filepath = "";
    bool initialized = false;
    glm::vec2 ybounds = glm::vec2(-1.0f, 1.0f);
    glm::vec2 xbounds = glm::vec2(-1.0f, 1.0f);
    glm::vec2 zbounds = glm::vec2(-1.0f, 1.0f);
    glm::vec3 color = glm::vec3(1.0f, 0.5f, 1.0f);
    bool mirror_horizontal = false;
    bool mirror_vertical = false;

    QOpenGLVertexArrayObject* body_VAO;
    QOpenGLBuffer* body_VBO;
    QOpenGLShaderProgram* body_shader = nullptr;
    QOpenGLVertexArrayObject* face_VAO;
    QOpenGLBuffer* face_VBO;
    QOpenGLShaderProgram* face_shader = nullptr;
    QImage* image;
    QOpenGLTexture* texture;

// this is messy, but easier than separate files
const char* vertex_source_body = "                           \n\
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
const char* fragment_source_body = "                         \n\
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

const char* vertex_source_face = "                      \n\
    #version 330 core                                   \n\
    layout (location = 0) in vec3 pos;                  \n\
    layout (location = 1) in vec3 nor;                  \n\
    layout (location = 2) in vec2 tex;                  \n\
    uniform mat4 transform;                             \n\
    uniform mat4 rotate;                                \n\
    out vec4 normal;                                    \n\
    out vec2 texcoord;                                  \n\
    void main(){                                        \n\
        gl_Position = transform * vec4(pos.xyz, 1.0f);  \n\
        normal = rotate * vec4(nor.xyz, 1.0f);          \n\
        texcoord = tex;                                 \n\
    }";
const char* fragment_source_face = "                    \n\
    #version 330 core                                   \n\
    in vec4 normal;                                     \n\
    in vec2 texcoord;                                   \n\
    uniform sampler2D image;                            \n\
    out vec4 FragColor;                                 \n\
    void main(){                                        \n\
        vec3 shade = texture(image, texcoord.xy).xyz;       \n\
        vec3 light1 = vec3(-0.70, 0.42, 0.58);          \n\
        float diff1 = max(dot(light1, normal.xyz), 0.0);\n\
        vec3 light2 = vec3(0.70, -0.42, -0.58);         \n\
        float diff2 = max(dot(light2, normal.xyz), 0.0);\n\
        float ambient = 0.1;                            \n\
        vec3 final = (ambient+2*(diff1+diff2*0.5))*shade;\n\
        //FragColor = vec4(final.xyz, 1.0f);              \n\
        //don't use fancy shading on images to preserve original colors\n\
        FragColor = vec4(shade.xyz, 1.0f);              \n\
    }";

Image()  {
    initializeOpenGLFunctions();
}

void initialize(QString filepath){
    this->filepath = filepath;
    if(filepath == ""){ return; }
    if(!(QFileInfo::exists(filepath) && QFileInfo(filepath).isFile())){
        QMessageBox::warning(nullptr, "File Reading Error", "File not found: \""+filepath+"\".");
        return;
    }

    image = new QImage();
    image->load(filepath);
    *image = image->mirrored(mirror_horizontal, mirror_vertical);
    texture = new QOpenGLTexture(*image);
    texture->setMagnificationFilter(QOpenGLTexture::Nearest);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);

    // TODO: assert lower < higher bounds

    // position, normal, texture
    float image_vertices[] = {
        xbounds[0], ybounds[0], zbounds[1], 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[1], 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[1], 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        xbounds[1], ybounds[1], zbounds[1], 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        xbounds[0], ybounds[1], zbounds[1], 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        xbounds[0], ybounds[0], zbounds[1], 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    };

    face_VAO = new QOpenGLVertexArrayObject();
    face_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    face_VAO->create();
    face_VAO->bind();
    face_VBO->create();
    face_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    face_VBO->bind();
    face_VBO->allocate(image_vertices, sizeof(float)*8*6);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    face_VAO->release();

    if(face_shader == nullptr){
        // shader won't compile vertex shader twice for some reason, so only do it once
        // TODO: figure out why
        face_shader = new QOpenGLShaderProgram();
        face_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source_face);
        face_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source_face);
        face_shader->link();
    }

    // position, normal
    float box_vertices[] = {
        // bottom
        xbounds[0], ybounds[0], zbounds[0], 0.0f, 0.0f, -1.0f,
        xbounds[1], ybounds[0], zbounds[0], 0.0f, 0.0f, -1.0f,
        xbounds[1], ybounds[1], zbounds[0], 0.0f, 0.0f, -1.0f,
        xbounds[1], ybounds[1], zbounds[0], 0.0f, 0.0f, -1.0f,
        xbounds[0], ybounds[1], zbounds[0], 0.0f, 0.0f, -1.0f,
        xbounds[0], ybounds[0], zbounds[0], 0.0f, 0.0f, -1.0f,
        // left
        xbounds[0], ybounds[0], zbounds[0], -1.0f, 0.0f, 0.0f,
        xbounds[0], ybounds[1], zbounds[0], -1.0f, 0.0f, 0.0f,
        xbounds[0], ybounds[1], zbounds[1], -1.0f, 0.0f, 0.0f,
        xbounds[0], ybounds[1], zbounds[1], -1.0f, 0.0f, 0.0f,
        xbounds[0], ybounds[0], zbounds[1], -1.0f, 0.0f, 0.0f,
        xbounds[0], ybounds[0], zbounds[0], -1.0f, 0.0f, 0.0f,
        // right
        xbounds[1], ybounds[0], zbounds[0], 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[0], 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[1], 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[1], 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[1], 1.0f, 0.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[0], 1.0f, 0.0f, 0.0f,
        // back
        xbounds[0], ybounds[0], zbounds[0], 0.0f, -1.0f, 0.0f,
        xbounds[0], ybounds[0], zbounds[1], 0.0f, -1.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[1], 0.0f, -1.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[1], 0.0f, -1.0f, 0.0f,
        xbounds[1], ybounds[0], zbounds[0], 0.0f, -1.0f, 0.0f,
        xbounds[0], ybounds[0], zbounds[0], 0.0f, -1.0f, 0.0f,
        // front
        xbounds[0], ybounds[1], zbounds[0], 0.0f, 1.0f, 0.0f,
        xbounds[0], ybounds[1], zbounds[1], 0.0f, 1.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[1], 0.0f, 1.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[1], 0.0f, 1.0f, 0.0f,
        xbounds[1], ybounds[1], zbounds[0], 0.0f, 1.0f, 0.0f,
        xbounds[0], ybounds[1], zbounds[0], 0.0f, 1.0f, 0.0f,
    };

    body_VAO = new QOpenGLVertexArrayObject();
    body_VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    body_VAO->create();
    body_VAO->bind();
    body_VBO->create();
    body_VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    body_VBO->bind();
    body_VBO->allocate(box_vertices, sizeof(float)*6*6*5);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    body_VAO->release();

    if(body_shader == nullptr){
        // shader won't compile vertex shader twice for some reason, so only do it once
        // TODO: figure out why
        body_shader = new QOpenGLShaderProgram();
        body_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_source_body);
        body_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_source_body);
        body_shader->link();
    }

    initialized = true;
}

void deinitialize(){
    initialized = false;
    delete body_VBO;
    delete body_VAO;
    delete face_VBO;
    delete face_VAO;
    delete image;
    delete texture;
}

// free GPU memory
~Image(){
    if(initialized){
        deinitialize();
    }
    delete body_shader;
    delete face_shader;
}

// draw mesh
void render(glm::mat4 view, glm::mat4 rotate){
    if(initialized){
        // TODO: rotate normals
        body_shader->bind();
        unsigned int matlocation = glGetUniformLocation(body_shader->programId(), "transform");
        glUniformMatrix4fv(matlocation, 1, GL_FALSE, glm::value_ptr(view));
        unsigned int rotlocation = glGetUniformLocation(body_shader->programId(), "rotate");
        glUniformMatrix4fv(rotlocation, 1, GL_FALSE, glm::value_ptr(rotate));
        unsigned int collocation = glGetUniformLocation(body_shader->programId(), "color");
        glUniform3fv(collocation, 1, glm::value_ptr(color));
        body_VAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6*6*5);
        body_VAO->release();

        // TODO: rotate normals
        texture->bind();
        face_shader->bind();
        matlocation = glGetUniformLocation(face_shader->programId(), "transform");
        glUniformMatrix4fv(matlocation, 1, GL_FALSE, glm::value_ptr(view));
        rotlocation = glGetUniformLocation(face_shader->programId(), "rotate");
        glUniformMatrix4fv(rotlocation, 1, GL_FALSE, glm::value_ptr(rotate));
        face_VAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        face_VAO->release();

    }
}

};

#endif // IMAGE_H
