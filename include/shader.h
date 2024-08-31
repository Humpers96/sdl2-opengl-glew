#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <fstream>
#include <string>

class shader
{
public:

    shader(GLuint gl_type, const char* source_file);

    const GLuint& get() const;
    bool check_compile();
    void destroy();

private:

    bool load_from_file(const char* source_file);

    GLuint shader_id;
    GLuint shader_type;
    std::string src;
};