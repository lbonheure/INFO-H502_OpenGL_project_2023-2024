#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;

out vec4 v_col;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){
    gl_Position = P*V*M*vec4(position, 1);
    v_col = vec4(normal*0.5 + 0.5, 1.0);
};