// This code is heavely inspired by the code of Etay Meiri in its tutorial https://github.com/emeiri/ogldev/tree/master

#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include "texture.h"


struct PBRMaterial
{
    float Roughness = 0.0f;
    bool IsMetal = false;
    glm::vec3 Color = glm::vec3(0.0f, 0.0f, 0.0f);
};


class Material {

 public:
    glm::vec3 AmbientColor = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 DiffuseColor = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 SpecularColor = glm::vec3(0.0f, 0.0f, 0.0f);

    PBRMaterial PBRmaterial;

    Texture* pDiffuse = NULL; // base color of the material
    Texture* pSpecularExponent = NULL;

    Material() {}

    ~Material()
    {
        if (pDiffuse)
        {
            free(pDiffuse);
        }
        if (pSpecularExponent)
        {
            free(pSpecularExponent);
        }
    }
};


#endif