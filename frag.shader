#version 330 core
out vec4 frag_col;

in vec4 vert_col;

uniform vec4 input_colour;

void main()
{
    frag_col = input_colour;
}