#version 150

smooth in vec4 vertex_color;

out vec4 output_color;

void main()
{
   output_color = vertex_color;
}