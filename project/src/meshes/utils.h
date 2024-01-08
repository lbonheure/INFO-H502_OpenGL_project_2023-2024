#ifndef SKIN_MESH_UTILS_H
#define SKIN_MESH_UTILS_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

using namespace Assimp;

#define COLOR_TEXTURE_UNIT GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX 0
#define SPECULAR_EXPONENT_UNIT GL_TEXTURE6
#define SPECULAR_EXPONENT_UNIT_INDEX 6


std::string getDirFromPath(const std::string& Filename)
{
    // Extract the directory part from the file name
    std::string::size_type SlashIndex;

#ifdef _WIN64
    SlashIndex = Filename.find_last_of("\\");

    if (SlashIndex == -1) {
        SlashIndex = Filename.find_last_of("/");
    }
#else
    SlashIndex = Filename.find_last_of("/");
#endif

    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    return Dir;
}


inline glm::mat4 assimpToGlmMatrix4x4(aiMatrix4x4 mat) 
{
	glm::mat4 m;
	for (int y = 0; y < 4; y++){
		for (int x = 0; x < 4; x++){
			m[x][y] = mat[y][x];
		}
	}
	return m;
}


inline glm::mat3 assimpToGlmMatrix3x3(aiMatrix3x3 mat) 
{
	glm::mat3 m;
	for (int y = 0; y < 3; y++){
		for (int x = 0; x < 3; x++){
			m[x][y] = mat[y][x];
		}
	}
	return m;
}


inline glm::vec3 assimpToGlmVec3(aiVector3D vec) 
{
	return glm::vec3(vec.x, vec.y, vec.z);
}


inline glm::quat assimpToGlmQuat(aiQuaternion quat)
{
	glm::quat q;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	q.w = quat.w;

	return q;
}

#endif