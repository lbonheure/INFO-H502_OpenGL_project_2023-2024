#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){
    vec4 PosL = vec4(position, 1.0);
    gl_Position = P*V*M * PosL;
    TexCoord0 = texCoord;
    Normal0 = normal;
    LocalPos0 = vec3(M*PosL);
};