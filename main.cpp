#include <SDL2/SDL.h> // SDL2
#include <GL/glew.h> // glew - OpenGL function loader
#include <SDL2/SDL_opengl.h> // SDL2 OpenGL implementation
#include <GL/glu.h> // OpenGL utilities

#include <iostream>
#include <vector>

#include "shaders.h"

#define LOSER false

template <typename T> 
const T* vec_addr(const std::vector<T>& v)
{
    if (v.size())
        return &v[0];
    else
        return nullptr;
}

struct rgba
{
    float r = 1.f;
    float g = 1.f;
    float b = 1.f;
    float a = 1.f;
};

struct vertex
{
    float x;
    float y;
    float z;

    //rgba col;
};

bool init_SDL_GL()
{
    // initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return true;

    // specify OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // specify OpenGL core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    return false;
}

bool check_shader_compile(GLuint* shader)
{
    int success;
    char msg[512];

    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(*shader, 512, NULL, msg);
        std::cout << "shader compilation failed" << std::endl;
        std::cout << msg << std::endl;

        return true;
    }

    return false;
}

bool check_shader_link(GLuint* program)
{
    int success;
    char msg[512];

    glGetShaderiv(*program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(*program, 512, NULL, msg);
        std::cout << "shader compilation failed" << std::endl;
        std::cout << msg << std::endl;

        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    // -- SDL/OpenGL objects
    // ---------------------

    // init pointers for SDL objects
    SDL_Window* sdl_window = nullptr;
    SDL_Surface* sdl_surface = nullptr;
    SDL_GLContext sdl_context;

    if (init_SDL_GL())
        return -1;

    // create window
    sdl_window = SDL_CreateWindow("galea is a chump", 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  900, 600, 
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // check window creation was successful
    if (!sdl_window)
        return -1;

    sdl_context = SDL_GL_CreateContext(sdl_window);

    // check context creation was successful
    if (!sdl_context)
        return -1;

    SDL_GL_MakeCurrent(sdl_window, sdl_context);
    glViewport(0, 0, 900, 600);

    glewExperimental = GL_TRUE; // idk

    if (glewInit())
        return -1;



    // OpenGL shaders
    // --------------

    const char* v_shader_source = shaders::vertex_shader();
    GLuint v_shader;

    // inits OpenGL shader object (same principle as GL_ARRAY_BUFFER etc) 
    v_shader = glCreateShader(GL_VERTEX_SHADER);

    // attach the vertex shader source code to the GL_VERTEX_SHADER object and compile
    // shader to compile, number of strings passed as source, location of source
    glShaderSource(v_shader, 1, &v_shader_source, NULL);
    glCompileShader(v_shader);

    // check shader compilation was successful
    if (check_shader_compile(&v_shader))
        return -1;
    
    // repeat for fragment shader
    const char* f_shader_source = shaders::fragment_shader();
    GLuint f_shader;

    f_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(f_shader, 1, &f_shader_source, NULL);
    glCompileShader(f_shader);

    if (check_shader_compile(&f_shader))
        return -1;

    // create a shader program to link vert/frag shaders to
    GLuint shader_program;
    shader_program = glCreateProgram();

    // attach the shaders to the shader program
    glAttachShader(shader_program, v_shader);
    glAttachShader(shader_program, f_shader);
    // link shaders, i.e route output of one shader to the input of the next
    glLinkProgram(shader_program);

    if (check_shader_link(&shader_program))
        return -1;
    
    // after shaders have been linked into shader program they can be deleted
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);



    // OpenGL buffer/array objects
    // ---------------------

    std::vector<vertex> verts
    {
        { -0.5f, -0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        {  0.0f,  0.5f, 0.0f }
    };

    // init vertex buffer obj & vertex array obj
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // bind VAO first and then bind & set vertex buffers
    glBindVertexArray(VAO);

    // binds VBO to the GL_ARRAY_BUFFER target
    // any calls directed at GL_ARRAY_BUFFER will target the currently bound buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // copies user-defined data into the GL_ARRAY_BUFFER
    // type of buffer to copy into, size, data
    // STATIC - data set once, drawn many times
    // DYNAMIC - data set many times, drawn many times
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 3, vec_addr(verts), GL_STATIC_DRAW);
    // at this point vertex data is stored on the gpu in memory managed by the vertex buffer VBO

    
    // OpenGL needs to know the context of the vertex data it is being fed
    // location of the vertex attribute to configure (location = 0)
    // size of attribute (vec3 so 3)
    // type of data (all GLSL vec* are floating points)
    // whether or not input data should be normalised
    // stride value between vertices
    // start position/offset of data in buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    // each vertex attribute takes its data from memory managed by a VBO
    // which VBO it takes from is determined by which is currently bound to GL_ARRAY_BUFFER
    // when glVertexAttribPointer() is called

    // enable the vertex attributes using vertex attribute location (location = 0)
    glEnableVertexAttribArray(0);
    





    bool malta = false;

    while (malta == LOSER)
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // instructs OpenGL to use the specified shader program for any following rendering calls
        glUseProgram(shader_program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(sdl_window);

        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                malta = true;
                break;
            }

            if (ev.key.keysym.sym == SDLK_ESCAPE)
            {
                malta = true;
                break;
            }
        }
    }

    // OpenGL shutdown
    // ---------------

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);

    // SDL shutdown
    // ------------

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}