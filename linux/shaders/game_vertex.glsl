#version 330 core

layout(location = 0) in vec2 a_position;

uniform vec2 texcoords[4];
uniform mat4 viewproj;
uniform mat4 model;
out vec2 texcoord;

void
main() {
	gl_Position = viewproj * vec4(a_position, 0, 1);
	texcoord    = texcoords[gl_VertexID];
}
