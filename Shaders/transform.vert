#version 150

precision highp float;

layout(location = 0) in vec3 position;

smooth out vec4 vertex_color;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

uniform vec4 color;

void main()
{
	//Multiply projection, model and view matrices.
	mat4 pvm = projection_matrix * view_matrix * model_matrix;
	//Set the final vertex position.
	gl_Position =  pvm * vec4(position, 1.0);

	//Set the color.
	vertex_color = color;
}
