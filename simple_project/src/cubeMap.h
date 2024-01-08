#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <map>

#include "shader.h"
#include "meshes/object.h"

class CubeMap
{
public:
    Shader cubeMapShader;
    Object cubeMapObject;
    GLuint cubeMapTexture;


    CubeMap()
    {
        const char sourceVCubeMap[] = PATH_TO_MY_GAME_SHADERS "/vertex_cubeMap.cpp";
	    const char sourceFCubeMap[] = PATH_TO_MY_GAME_SHADERS "/fragment_cubeMap.cpp";

        this->cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap);

        char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
        this->cubeMapObject = Object(pathCube);
        this->cubeMapObject.makeObject(this->cubeMapShader);
    }


    CubeMap(const char* sourceVCubeMap, const char* sourceFCubeMap, const char* pathCube)
    {
        this->cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap);
        this->cubeMapObject = Object(pathCube);
        this->cubeMapObject.makeObject(this->cubeMapShader);
    }


    void loadTexture(std::string pathToCubeMap)
    {
        glGenTextures(1, &cubeMapTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

        // texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //stbi_set_flip_vertically_on_load(true);

        //std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/night/";//"/cubemaps/yokohama3/";//"/cubemaps/sky2/";//"/cubemaps/yokohama3/";

        std::map<std::string, GLenum> facesToLoad = {
            
            {pathToCubeMap + "px.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
            {pathToCubeMap + "py.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
            {pathToCubeMap + "pz.jpg",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
            {pathToCubeMap + "nx.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
            {pathToCubeMap + "ny.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
            {pathToCubeMap + "nz.jpg",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
            
        };
        //load the six faces
        for (std::pair<std::string, GLenum> pair : facesToLoad) {
            this->loadCubemapFace(pair.first.c_str(), pair.second);
        }
    }


    void loadCubemapFace(const char * path, const GLenum& targetFace)
    {
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {

            glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //glGenerateMipmap(targetFace);
        }
        else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << (reason == NULL ? "Probably not implemented by the student" : reason) << std::endl;
        }
        stbi_image_free(data);
    }


    void render(glm::mat4 view, glm::mat4 perspective)
    {
        this->cubeMapShader.use();
        this->cubeMapShader.setMatrix4("V", view);
        this->cubeMapShader.setMatrix4("P", perspective);
        this->cubeMapShader.setInteger("cubemapTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapTexture);
        this->cubeMapObject.draw();
        glDepthFunc(GL_LESS);
    }

};

#endif