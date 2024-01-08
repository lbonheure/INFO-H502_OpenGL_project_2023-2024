#ifndef STATIC_OBJECT_H
#define STATIC_OBJECT_H

#include <iostream>
#include <vector>

// Assimp library to load the mesh file
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"
#include "material.h"
#include "texture.h"
#include "world_transform.h"

using namespace Assimp;

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2

class StaticObject
{
private:
    #define INVALID_MATERIAL 0xFFFFFFFF

    enum BUFFER_TYPE {
        INDEX_BUFFER  = 0,
        POS_VB        = 1,
        TEXCOORD_VB   = 2,
        NORMAL_VB     = 3,
        NUM_BUFFERS   = 4
    };

    WorldTrans m_worldTransform;

    // VAO and VBO
    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    struct BasicMeshEntry {
        BasicMeshEntry()
        {
            NumIndices = 0;
            NumVertices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        unsigned int NumIndices;
        unsigned int NumVertices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    Assimp::Importer importer;  // the assimp importer
    const aiScene* scene;   // the assimp scene
    std::vector<BasicMeshEntry> m_Meshes;   // meshes
    std::vector<Material> m_Materials;  //materials

    // Temporary space for vertex stuff before we load them into the GPU
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;


public:
    StaticObject() {}

    ~StaticObject()
    {
        Clear();
    }

    /**
     * @brief Clear VAO and VBO
     * 
     */
    void Clear()
    {
        if (m_Buffers[0] != 0) {
            glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
        }

        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
    }

    WorldTrans& getWorldTransform() { return m_worldTransform; }

    /**
     * @brief Load meshes from the file in path
     * 
     * @param path the path of the file to load
     */
    void LoadMesh(const char* path)
    {
        // Release the previously loaded mesh (if it exists)
        Clear();

        // Create the VAO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        // Create the buffers for the vertices attributes
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

        // Import the file content with the assimp library
        scene = importer.ReadFile(path, 
								aiProcess_Triangulate  				|
                                aiProcess_GenNormals                |
								aiProcess_JoinIdenticalVertices		|
								aiProcess_ValidateDataStructure);

        if (scene) {
            initFromScene(path);
        }
        else {
            std::cout << "Error parsing " << path << ": " << importer.GetErrorString() << std::endl;
        }
    }

    void initFromScene(const char* path)
    {
        m_Meshes.resize(scene->mNumMeshes);
        m_Materials.resize(scene->mNumMaterials);

        unsigned int NumVertices = 0;
        unsigned int NumIndices = 0;

        countVerticesAndIndices(NumVertices, NumIndices);

        reserveSpace(NumVertices, NumIndices);

        initAllMeshes();
        initMaterials(path);

        populateBuffers();
    }

    void countVerticesAndIndices(unsigned int& NumVertices, unsigned int& NumIndices)
    {
        for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
            m_Meshes[i].MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
            m_Meshes[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
            m_Meshes[i].NumVertices = scene->mMeshes[i]->mNumVertices;
            m_Meshes[i].BaseVertex = NumVertices;
            m_Meshes[i].BaseIndex = NumIndices;

            NumVertices += scene->mMeshes[i]->mNumVertices;
            NumIndices  += m_Meshes[i].NumIndices;
        }
    }

    void reserveSpace(unsigned int NumVertices, unsigned int NumIndices)
    {
        m_Positions.reserve(NumVertices);
        m_Normals.reserve(NumVertices);
        m_TexCoords.reserve(NumVertices);
        m_Indices.reserve(NumIndices);
    }

    void initAllMeshes()
    {
        for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
            const aiMesh* mesh = scene->mMeshes[i];
            initSingleMesh(i, mesh);
        }
    }

    void initSingleMesh(uint meshIndex, const aiMesh* mesh)
    {
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        // Populate the vertex attribute vectors
        for (unsigned int i = 0 ; i < mesh->mNumVertices ; i++) {

            const aiVector3D& pos = mesh->mVertices[i];
            m_Positions.push_back(glm::vec3(pos.x, pos.y, pos.z));

            if (mesh->mNormals) {
                const aiVector3D& normal = mesh->mNormals[i];
                m_Normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
            } else {
                m_Normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            }

            const aiVector3D& texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : Zero3D;
            m_TexCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
        }

        // Populate the index buffer
        for (unsigned int i = 0 ; i < mesh->mNumFaces ; i++) {
            const aiFace& Face = mesh->mFaces[i];
            //        printf("num indices %d\n", Face.mNumIndices);
            //        assert(Face.mNumIndices == 3);
            m_Indices.push_back(Face.mIndices[0]);
            m_Indices.push_back(Face.mIndices[1]);
            m_Indices.push_back(Face.mIndices[2]);
        }
    }


    void initMaterials(const char* path)
    {
        std::string directory = getDirFromPath(path);

        // Initialize the materials
        for (unsigned int i = 0 ; i < scene->mNumMaterials ; i++) {
            const aiMaterial* material = scene->mMaterials[i];

            loadTextures(directory, material, i);

            loadColors(material, i);
        }
    }


    void loadTextures(const std::string& directory, const aiMaterial* material, int index)
    {
        loadDiffuseTexture(directory, material, index);
        loadSpecularTexture(directory, material, index);
    }
    
    void loadDiffuseTexture(const std::string& directory, const aiMaterial* material, int index)
    {
        m_Materials[index].pDiffuse = NULL;

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string p(Path.data);

                if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

                std::string FullPath = directory + "/" + p;

                m_Materials[index].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Materials[index].pDiffuse->Load()) {
                    std::cout << "Error loading diffuse texture " << FullPath.c_str() << std::endl;
                    exit(0);
                }
                else {
                    //std::cout << "Loaded diffuse texture " << FullPath.c_str() << std::endl;
                }
            }
        }
    }
    
    void loadSpecularTexture(const std::string& directory, const aiMaterial* material, int index)
    {
        m_Materials[index].pSpecularExponent = NULL;

        if (material->GetTextureCount(aiTextureType_SHININESS) > 0) {
            aiString Path;

            if (material->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string p(Path.data);

                if (p == "C:\\\\") {
                    p = "";
                } else if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

                std::string FullPath = directory + "/" + p;

                m_Materials[index].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Materials[index].pSpecularExponent->Load()) {
                    std::cout << "Error loading specular texture " << FullPath.c_str() << std::endl;
                    exit(0);
                }
                else {
                    //std::cout << "Loaded specular texture " << FullPath.c_str() << std::endl;
                }
            }
        }
    }
    
    void loadColors(const aiMaterial* material, int index)
    {
        aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);

        int ShadingModel = 0;
        if (material->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
            //std::cout << "Shading model " << ShadingModel << std::endl;
        }

        if (material->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
            //std::cout << "Loaded ambient color [" << AmbientColor.r << " " << AmbientColor.g << " " << AmbientColor.b << "]" << std::endl;
            m_Materials[index].AmbientColor.r = AmbientColor.r;
            m_Materials[index].AmbientColor.g = AmbientColor.g;
            m_Materials[index].AmbientColor.b = AmbientColor.b;
        } else {
            m_Materials[index].AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);

        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
            //std::cout << "Loaded diffuse color [" << DiffuseColor.r << " " << DiffuseColor.g << " " << DiffuseColor.b << "]" << std::endl;
            m_Materials[index].DiffuseColor.r = DiffuseColor.r;
            m_Materials[index].DiffuseColor.g = DiffuseColor.g;
            m_Materials[index].DiffuseColor.b = DiffuseColor.b;
        }

        aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);

        if (material->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
            //std::cout << "Loaded specular color [" << SpecularColor.r << " " << SpecularColor.g << " " << SpecularColor.b << "]" << std::endl;
            m_Materials[index].SpecularColor.r = SpecularColor.r;
            m_Materials[index].SpecularColor.g = SpecularColor.g;
            m_Materials[index].SpecularColor.b = SpecularColor.b;
        }
    }


    void populateBuffers()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, false, 0, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, false, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, false, 0, 0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);

        //desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
    }


    /**
     * @brief Render the object in the screen
     * 
     */
    void render()
    {
        glBindVertexArray(m_VAO);

        for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
            unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

            assert(MaterialIndex < m_Materials.size());

            if (m_Materials[MaterialIndex].pDiffuse) {
                m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
            }

            if (m_Materials[MaterialIndex].pSpecularExponent) {
                m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
            }

            glDrawElementsBaseVertex(GL_TRIANGLES,
                                    m_Meshes[i].NumIndices,
                                    GL_UNSIGNED_INT,
                                    (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                                    m_Meshes[i].BaseVertex);
        }

        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
    }


    const Material& getMaterial()
    {
        for (unsigned int i = 0 ; i < m_Materials.size() ; i++) {
            if (m_Materials[i].AmbientColor != glm::vec3(0.0f, 0.0f, 0.0f)) {
                return m_Materials[i];
            }
        }

        return m_Materials[0];
    }


};



#endif