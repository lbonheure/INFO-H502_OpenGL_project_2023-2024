/*

        Copyright 2021 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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