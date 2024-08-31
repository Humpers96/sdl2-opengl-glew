#version 330 core
layout (location = 0) in vec3 vert_pos;

out vec4 vert_col;

void main()
{
    vert_col = vec4(0.33, 0.0, 0.0, 1.0);
    gl_Position = vec4(vert_pos.x, vert_pos.y, vert_pos.z, 1.0);
}