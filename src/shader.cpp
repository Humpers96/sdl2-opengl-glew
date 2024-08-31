#include "shader.h"

#include <fstream>
#include <string>
#include <iostream>

shader::shader(GLuint gl_type, const char* source_file) :
    shader_type{ gl_type }
{
    // inits OpenGL shader object (same principle as GL_ARRAY_BUFFER etc) 
    shader_id = glCreateShader(gl_type);
    load_from_file(source_file);

    const char* source = src.c_str();
    // attach the vertex shader source code to the GL_[VERTEX | FRAGMENT]_SHADER object
    // shader, number of strings passed as source, location of source
    glShaderSource(shader_id, 1, &source, NULL);

    // compile shader
    glCompileShader(shader_id);
}

const GLuint& shader::get() const
{
    return shader_id;
}

bool shader::check_compile()
{
    // check for compilation errors
    int success;
    char msg[256];

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader_id, 512, NULL, msg);
        std::cout << "shader compilation failed" << std::endl;
        std::cout << msg << std::endl;

        return true;
    }

    return false;
}

void shader::destroy()
{
    // delete shader
    glDeleteShader(shader_id);
}

bool shader::load_from_file(const char* source_file)
{
    // read in shader source from file
    std::ifstream file(source_file, std::ios::in);

    if (!file.is_open())
        return true;

    std::string line;

    while (std::getline(file, line))
    {
        src += line + "\n";
    }

    file.close();

    return false;
}