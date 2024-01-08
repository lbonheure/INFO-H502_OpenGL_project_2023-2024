/*

        Copyright 2011 Etay Meiri

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

// source of this code: https://github.com/emeiri/ogldev/tree/master

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


class Texture
{
private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
    
    void LoadInternal(void* image_data)
    {
        glGenTextures(1, &m_textureObj);
        glBindTexture(m_textureTarget, m_textureObj);

        if (m_textureTarget == GL_TEXTURE_2D) {
            switch (m_imageBPP) {
            case 1:
                glTexImage2D(m_textureTarget, 0, GL_RED, m_imageWidth, m_imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, image_data);
                break;

            case 2:
                glTexImage2D(m_textureTarget, 0, GL_RG, m_imageWidth, m_imageHeight, 0, GL_RG, GL_UNSIGNED_BYTE, image_data);
                break;

            case 3:
                glTexImage2D(m_textureTarget, 0, GL_RGB, m_imageWidth, m_imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
                break;

            case 4:
                glTexImage2D(m_textureTarget, 0, GL_RGBA, m_imageWidth, m_imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
                break;

            //default:
                //NOT_IMPLEMENTED;
            }
        } else {
            std::cout << "Support for texture target " << m_textureTarget << " is not implemented" << std::endl;
            exit(1);
        }

        glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(m_textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenerateMipmap(m_textureTarget);

        glBindTexture(m_textureTarget, 0);
    }

public:
    Texture(GLenum TextureTarget, const std::string& FileName)
    {
        m_textureTarget = TextureTarget;
        m_fileName      = FileName;
    }

    Texture(GLenum TextureTarget)
    {
        m_textureTarget = TextureTarget;
    }

    // Should be called once to load the texture
    bool Load()
    {
        stbi_set_flip_vertically_on_load(1);
        unsigned char* image_data = stbi_load(m_fileName.c_str(), &m_imageWidth, &m_imageHeight, &m_imageBPP, 0);
        if (!image_data) {
            std::cout << "Can't load texture from '" << m_fileName.c_str() << "' - " << stbi_failure_reason() << std::endl;
            exit(0);
        }
        //std::cout << "Width " << m_imageWidth << ", height " << m_imageHeight << ", bpp " << m_imageBPP << std::endl;
        LoadInternal(image_data);
        return true;
    }

    void Load(unsigned int BufferSize, void* pData)
    {
        void* image_data = stbi_load_from_memory((const stbi_uc*)pData, BufferSize, &m_imageWidth, &m_imageHeight, &m_imageBPP, 0);
        LoadInternal(image_data);
        stbi_image_free(image_data);
    }

    void Load(const std::string& Filename)
    {
        m_fileName = Filename;
        if (!Load()) {
            exit(0);
        }
    }

    void LoadRaw(int Width, int Height, int BPP, unsigned char* pData)
    {
        m_imageWidth = Width;
        m_imageHeight = Height;
        m_imageBPP = BPP;

        LoadInternal(pData);
    }

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit)
    {
        glActiveTexture(TextureUnit);
        glBindTexture(m_textureTarget, m_textureObj);
    }

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

};


#endif  /* TEXTURE_H */
