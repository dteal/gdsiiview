#ifndef SHADER_H
#define SHADER_H

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

// object to help manage an OpenGL shader
class Shader{
private:
GLuint program;

public:
// create a shader given vertex and fragment shader source code
Shader(const char* vertex_source, const char* fragment_source){

    // compile vertex shader
    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);
    int success;
    char info[1024];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertex_shader, 1024, NULL, info);
        std::cout << "Error compiling vertex shader: " << info << std::endl;
    }

    // compile fragment shader
    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragment_shader, 1024, NULL, info);
        std::cout << "Error compiling fragment shader: " << info << std::endl;
    }

    // link shader program
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(vertex_shader, 1024, NULL, info);
        std::cout << "Error linking shader program: " << info << std::endl;
    }

    // free GPU memory
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

// free GPU memory
~Shader(){
    glDeleteProgram(program);
}

// enable shader; to be used before rendering geometry
void use(){
    glUseProgram(program);
}

// send vector to GPU
void set_vec3(const char* name, glm::vec3 vec){
    GLuint location = glGetUniformLocation(program, name);
    glUniform3fv(location, 1, glm::value_ptr(vec));
}

// send matrix to GPU
void set_mat4(const char* name, glm::mat4 mat){
    GLuint location = glGetUniformLocation(program, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

};

#endif
