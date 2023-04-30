#version 330 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texcoord;

uniform mat4 projview;

out vec2 texcoord;

void
main() {
	gl_Position = projview * vec4(a_position, 0, 1);
	texcoord = a_texcoord;
}
