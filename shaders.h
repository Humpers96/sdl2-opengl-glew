#include <fstream>
#include <string>

namespace
{
    bool init = false;
    std::string vertex_shader_;
    std::string fragment_shader_;
}

namespace shaders
{
    bool load_shaders()
    {
        std::ifstream file("vert.shader", std::ios::in);

        if (!file.is_open())
            return true;

        std::string line;

        while (std::getline(file, line))
        {
            vertex_shader_ += line + "\n";
        }

        file.close();
        file.open("frag.shader", std::ios::in);

        if (!file.is_open())
            return true;

        line.clear();

        while (std::getline(file, line))
        {
            fragment_shader_ += line + "\n";
        }

        init = true;
        return false;
    }

    static const char* vertex_shader()
    {
        if (!init)
            load_shaders();

        return vertex_shader_.c_str();
    }

    static const char* fragment_shader()
    {
        if (!init)
            load_shaders();

        return fragment_shader_.c_str();
    }
}