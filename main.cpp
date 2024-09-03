#include <SDL2/SDL.h> // SDL2
#include <GL/glew.h> // glew - OpenGL function loader
#include <SDL2/SDL_opengl.h> // SDL2 OpenGL implementation
#include <GL/glu.h> // OpenGL utilities

#include <iostream>
#include <vector>
#include <math.h>

#include "shader.h"

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

    void print() const
    {
        std::cout << "x: " << x << ", y: " << y << ", z: " << z << std::endl;
    }
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
    // SDL/OpenGL objects
    // ------------------

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

    shader vert_shader(GL_VERTEX_SHADER, "vert.shader");
    shader frag_shader(GL_FRAGMENT_SHADER, "frag.shader");

    if (vert_shader.check_compile())
        return -1;
        
    if (frag_shader.check_compile())
        return -1;

    // create a shader program to link vert/frag shaders to
    GLuint shader_program;
    shader_program = glCreateProgram();

    // attach the shaders to the shader program
    glAttachShader(shader_program, vert_shader.get());
    glAttachShader(shader_program, frag_shader.get());
    // link shaders, i.e route output of one shader to the input of the next
    glLinkProgram(shader_program);

    if (check_shader_link(&shader_program))
        return -1;
    
    // after shaders have been linked into shader program they can be deleted
    vert_shader.destroy();
    frag_shader.destroy();



    // OpenGL buffer/array objects
    // ---------------------

    std::vector<vertex> verts = 
    {
        {  0.5f,  0.5f, 0.0f }, // top right
        {  0.5f, -0.5f, 0.0f }, // bottom right
        { -0.5f, -0.5f, 0.0f }, // bottom left
        { -0.5f,  0.5f, 0.0f }  // top left
    };
    
    unsigned int indices[] = 
    {
        0, 1, 3,
        1, 2, 3
    };

    // init vertex buffer, element buffer obj & vertex array obj
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind VAO first and then bind & set vertex buffers
    glBindVertexArray(VAO);

    // binds VBO to the GL_ARRAY_BUFFER target
    // any calls directed at GL_ARRAY_BUFFER will target the currently bound buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // copies user-defined data into the GL_ARRAY_BUFFER
    // type of buffer to copy into, size, data
    // STATIC - data set once, drawn many times
    // DYNAMIC - data set many times, drawn many times
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * verts.size(), vec_addr(verts), GL_STATIC_DRAW);
    // at this point vertex data is stored on the gpu in memory managed by the vertex buffer VBO

    // binds EBO to the GL_ELEMENT_ARRAY_BUFFER target
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // OpenGL needs to know the context of the vertex data it is being fed
    // location of the vertex attribute to configure (location = 0)
    // size of attribute (vec3 so 3)
    // type of data (all GLSL vec* are floating points)
    // whether or not input data should be normalised
    // stride value between vertices
    // start position/offset of data in buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    // each vertex attribute takes its data from memory managed by a VBO
    // which VBO it takes from is determined by which is currently bound to GL_ARRAY_BUFFER
    // when glVertexAttribPointer() is called

    // enable the vertex attributes using vertex attribute location (location = 0)
    glEnableVertexAttribArray(0);

    // GL_ARRAY_BUFFER (VBO) can be unbound as glVertexAttribPointer registered to the vertex attribute the relevant buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // unbind VAO in order to prevent accidentally manipulating it at a later point
    // should be rebound before drawing
    glBindVertexArray(0);



    // rendering loop
    // --------------

    int colour_uniform = glGetUniformLocation(shader_program, "input_colour");

    bool malta = false;

    while (malta == LOSER)
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
        // glFrontFace(GL_CW);

        // instructs OpenGL to use the specified shader program for any following rendering calls
        glUseProgram(shader_program);
        // bind VAO to draw from
        glBindVertexArray(VAO);

        float ticks = (SDL_GetTicks() / 500.f);

        float red = sin(ticks);
        float green = -red;

        glUniform4f(colour_uniform, red, green, 0.0f, 1.0f);

        // draw arrays
        // OpenGL type to draw
        // starting vertex index in array
        // number of vertices to draw
        //glDrawArrays(GL_TRIANGLES, 0, verts.size());
        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

        // swap buffers
        SDL_GL_SwapWindow(sdl_window);

        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
                case SDL_KEYDOWN:
                {
                    switch (ev.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        {
                            malta = true;
                            std::cout << "escape pressed" << std::endl;
                            break;
                        }
                        case SDLK_m:
                        {
                            std::cout << "malta man bad" << std::endl;
                            break;
                        }
                        case SDLK_g:
                        {
                            std::cout << "galil" << std::endl;
                            break;
                        }
                        default:
                        {
                            std::cout << "key pressed" << std::endl;
                            break;
                        }
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    std::cout << "click" << std::endl;
                    break;
                }
                case SDL_QUIT:
                {
                    malta = true;
                    std::cout << "sdl quit called" << std::endl;
                    break;
                }
            }
        }
    }



    // OpenGL shutdown
    // ---------------

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);



    // SDL shutdown
    // ------------

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}