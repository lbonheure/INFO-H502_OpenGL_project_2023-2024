#version 440 core

// This shader is inpired of the vertex shaders of Etay Meiri in its tutorial (https://github.com/emeiri/ogldev/blob/master/tutorial28_youtube/skinning.vs)
// and of hasinaxp (https://github.com/hasinaxp/skeletal_animation-_assimp_opengl/blob/master/main.cpp)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec4 BoneIDs0_3;
layout (location = 4) in vec4 BoneIDs4_7;
layout (location = 5) in vec2 BoneIDs8_9;
layout (location = 6) in vec4 Weights0_3;
layout (location = 7) in vec4 Weights4_7;
layout (location = 8) in vec2 Weights8_9;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;

const int MAX_BONES = 100;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 gBones[MAX_BONES];

void main(){
    mat4 boneTransform = mat4(0.0);
    boneTransform += gBones[int(BoneIDs0_3.x)] * Weights0_3.x;
	boneTransform += gBones[int(BoneIDs0_3.y)] * Weights0_3.y;
    boneTransform += gBones[int(BoneIDs0_3.z)] * Weights0_3.z;
    boneTransform += gBones[int(BoneIDs0_3.w)] * Weights0_3.w;

    boneTransform += gBones[int(BoneIDs4_7.x)] * Weights4_7.x;
    boneTransform += gBones[int(BoneIDs4_7.y)] * Weights4_7.y;
    boneTransform += gBones[int(BoneIDs4_7.z)] * Weights4_7.z;
    boneTransform += gBones[int(BoneIDs4_7.w)] * Weights4_7.w;

    boneTransform += gBones[int(BoneIDs8_9.x)] * Weights8_9.x;
    boneTransform += gBones[int(BoneIDs8_9.y)] * Weights8_9.y;
    //if (boneTransform == mat4(0.0)) boneTransform = mat4(1.0);

    vec4 PosL = boneTransform * vec4(position, 1.0);
    gl_Position = P*V*M * PosL;
    TexCoord0 = texCoord;
    Normal0 = normal;
    LocalPos0 = position;//vec3(M*PosL);
}
