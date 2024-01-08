// This code is heavely inspired by the code of Etay Meiri in its tutorial (https://github.com/emeiri/ogldev/tree/master/tutorial28_youtube)

#ifndef ANIMATED_OBJECT_H
#define ANIMATED_OBJECT_H

#include <iostream>
#include <map>
#include <vector>
#include <unordered_map>

// Assimp library to load the mesh file
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>

#include "../shader.h"

#include "utils.h"
#include "material.h"
#include "texture.h"
#include "world_transform.h"

using namespace Assimp;

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 6


class AnimatedObject
{
private:
    #define MAX_NUM_BONES_PER_VERTEX 10
    #define INVALID_MATERIAL 0xFFFFFFFF

    struct VertexBoneData
    {
        float BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0 };

        VertexBoneData()
        {
        }

        void AddBoneData(uint BoneID, float Weight)
        {
            for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(BoneIDs) ; i++) {
                if (Weights[i] == 0.0) {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }

            // should never get here - more bones than we have space for
            assert(0);
        }
    };

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB       = 1,
        TEXCOORD_VB  = 2,
        NORMAL_VB    = 3,
        BONE_VB      = 4,
        NUM_BUFFERS  = 5
    };

    WorldTrans m_worldTransform;
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
    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<Material> m_Materials;

    // Temporary space for vertex stuff before we load them into the GPU
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;
    std::vector<VertexBoneData> m_Bones;

    std::map<std::string,uint> m_BoneNameToIndexMap;

    struct BoneInfo
    {
        uint id;
        glm::mat4 OffsetMatrix;
        glm::mat4 FinalTransformation;

        BoneInfo(const glm::mat4& Offset, uint boneId)
        {
            id = boneId;
            OffsetMatrix = Offset;
            FinalTransformation = glm::mat4(0.0f);
        }
    };

    std::vector<BoneInfo> m_BoneInfo;
    glm::mat4 m_GlobalInverseTransform;

public:
    AnimatedObject() {};

    ~AnimatedObject() 
    {
        Clear();
    };

    /**
     * @brief Clear the buffers for loaded mesh
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
            m_GlobalInverseTransform = assimpToGlmMatrix4x4(scene->mRootNode->mTransformation);
            m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
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

            //if (i < 3) std::cout << "numVertices and numIndices " << NumVertices << " " << NumIndices << std::endl;

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
        m_Bones.resize(NumVertices);
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

        loadMeshBones(meshIndex, mesh);

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
    
    void loadMeshBones(uint meshIndex, const aiMesh* mesh)
    {
        for (uint i = 0 ; i < mesh->mNumBones ; i++) {
            loadSingleBone(meshIndex, mesh->mBones[i]);
        }
    }
    
    void loadSingleBone(uint meshIndex, const aiBone* bone)
    {
        int BoneId = getBoneId(bone);

        if (BoneId == m_BoneInfo.size()) {
            BoneInfo bi(assimpToGlmMatrix4x4(bone->mOffsetMatrix), BoneId);
            m_BoneInfo.push_back(bi);
        }

        for (uint i = 0 ; i < bone->mNumWeights ; i++) {
            const aiVertexWeight& vw = bone->mWeights[i];
            uint GlobalVertexID = m_Meshes[meshIndex].BaseVertex + bone->mWeights[i].mVertexId;
            m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
        }
    }


    int getBoneId(const aiBone* bone)
    {
        int BoneIndex = 0;
        std::string BoneName(bone->mName.C_Str());

        if (m_BoneNameToIndexMap.find(BoneName) == m_BoneNameToIndexMap.end()) {
            // Allocate an index for a new bone
            BoneIndex = (int)m_BoneNameToIndexMap.size();
            m_BoneNameToIndexMap[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneNameToIndexMap[BoneName];
        }

        return BoneIndex;
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

        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), &m_Bones[0], GL_STATIC_DRAW);


        glEnableVertexAttribArray(BONE_ID_LOCATION);
        glVertexAttribPointer(BONE_ID_LOCATION, 4, GL_FLOAT, false, sizeof(VertexBoneData), (void*)0);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, false, sizeof(VertexBoneData), (void*)(4 * sizeof(int32_t)));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, false, sizeof(VertexBoneData), (void*)(8 * sizeof(int32_t)));

        glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
        glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, false, sizeof(VertexBoneData),
                            (void*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(VertexBoneData),
                            (void*)((MAX_NUM_BONES_PER_VERTEX) * sizeof(int32_t) + 4*sizeof(float)));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 2, GL_FLOAT, false, sizeof(VertexBoneData),
                            (void*)((MAX_NUM_BONES_PER_VERTEX) * sizeof(int32_t) + 8*sizeof(float)));
        
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

    
    uint findPosition(float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        for (uint i = 0 ; i < nodeAnim->mNumPositionKeys - 1 ; i++) {
            float t = (float)nodeAnim->mPositionKeys[i + 1].mTime;
            if (AnimationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }

    
    void calcInterpolatedPosition(glm::vec3& Out, float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        // we need at least two values to interpolate...
        if (nodeAnim->mNumPositionKeys == 1) {
            Out = assimpToGlmVec3(nodeAnim->mPositionKeys[0].mValue);
            return;
        }

        uint PositionIndex = findPosition(AnimationTimeTicks, nodeAnim);
        uint NextPositionIndex = PositionIndex + 1;
        assert(NextPositionIndex < nodeAnim->mNumPositionKeys);
        float t1 = (float)nodeAnim->mPositionKeys[PositionIndex].mTime;
        float t2 = (float)nodeAnim->mPositionKeys[NextPositionIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);

        glm::vec3 Start = assimpToGlmVec3(nodeAnim->mPositionKeys[PositionIndex].mValue);
        glm::vec3 End = assimpToGlmVec3(nodeAnim->mPositionKeys[NextPositionIndex].mValue);
        Out = glm::mix(Start, End, Factor);
    }

    
    uint findRotation(float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        assert(nodeAnim->mNumRotationKeys > 0);

        for (uint i = 0 ; i < nodeAnim->mNumRotationKeys - 1 ; i++) {
            float t = (float)nodeAnim->mRotationKeys[i + 1].mTime;
            if (AnimationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }

    
    void calcInterpolatedRotation(glm::quat& Out, float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        // we need at least two values to interpolate...
        if (nodeAnim->mNumRotationKeys == 1) {
            Out = assimpToGlmQuat(nodeAnim->mRotationKeys[0].mValue);
            return;
        }

        uint RotationIndex = findRotation(AnimationTimeTicks, nodeAnim);
        uint NextRotationIndex = RotationIndex + 1;
        assert(NextRotationIndex < nodeAnim->mNumRotationKeys);
        float t1 = (float)nodeAnim->mRotationKeys[RotationIndex].mTime;
        float t2 = (float)nodeAnim->mRotationKeys[NextRotationIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        glm::quat StartRotationQ = assimpToGlmQuat(nodeAnim->mRotationKeys[RotationIndex].mValue);
	    glm::quat EndRotationQ = assimpToGlmQuat(nodeAnim->mRotationKeys[NextRotationIndex].mValue);
        Out = glm::slerp(StartRotationQ, EndRotationQ, Factor);
    }

    
    uint findScaling(float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        assert(nodeAnim->mNumScalingKeys > 0);

        
        for (uint i = 0 ; i < nodeAnim->mNumScalingKeys - 1 ; i++) {
            float t = (float)nodeAnim->mScalingKeys[i + 1].mTime;
            if (AnimationTimeTicks < t) {
                return i;
            }
        }
        return 0;
    }

    
    void calcInterpolatedScaling(glm::vec3& Out, float AnimationTimeTicks, const aiNodeAnim* nodeAnim)
    {
        // we need at least two values to interpolate...
        if (nodeAnim->mNumScalingKeys == 1) {
            Out = assimpToGlmVec3(nodeAnim->mScalingKeys[0].mValue);
            return;
        }

        uint ScalingIndex = findScaling(AnimationTimeTicks, nodeAnim);
        uint NextScalingIndex = ScalingIndex + 1;
        assert(NextScalingIndex < nodeAnim->mNumScalingKeys);
        float t1 = (float)nodeAnim->mScalingKeys[ScalingIndex].mTime;
        float t2 = (float)nodeAnim->mScalingKeys[NextScalingIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
        if (Factor < 0.0f || Factor > 1.0f){
            std::cout << "Factor " << Factor << " t1 " << t1 << " t2 " << t2 << " deltaTime " << DeltaTime << " animationTicks " << AnimationTimeTicks << std::endl;
        }
        assert(Factor >= 0.0f && Factor <= 1.0f);
        glm::vec3 Start = assimpToGlmVec3(nodeAnim->mScalingKeys[ScalingIndex].mValue);
        glm::vec3 End = assimpToGlmVec3(nodeAnim->mScalingKeys[NextScalingIndex].mValue);
        Out = glm::mix(Start, End, Factor);
    }

    
    void readNodeHierarchy(float AnimationTimeTicks, const aiNode* node, const glm::mat4& parentTransform)
    {
        std::string NodeName(node->mName.data);

        const aiAnimation* animation = scene->mAnimations[0];

        glm::mat4 NodeTransformation = assimpToGlmMatrix4x4(node->mTransformation);

        const aiNodeAnim* nodeAnim = findNodeAnim(animation, NodeName);

        
        if (nodeAnim) {
            // Interpolate scaling and generate scaling transformation matrix
            glm::vec3 Scaling;
            calcInterpolatedScaling(Scaling, AnimationTimeTicks, nodeAnim);

            glm::mat4 ScalingM = glm::mat4(1.0f);
            ScalingM = glm::scale(ScalingM, Scaling);

            // Interpolate rotation and generate rotation transformation matrix
            glm::quat RotationQ;
            calcInterpolatedRotation(RotationQ, AnimationTimeTicks, nodeAnim);
            glm::mat4 RotationM = glm::toMat4(RotationQ);
            
            // Interpolate translation and generate translation transformation matrix
            glm::vec3 Translation;
            calcInterpolatedPosition(Translation, AnimationTimeTicks, nodeAnim);
            glm::mat4 TranslationM = glm::mat4(1.0f);
            TranslationM = glm::translate(TranslationM, Translation);

            // Combine the above transformations
            NodeTransformation = TranslationM * RotationM * ScalingM;
        }
        

        glm::mat4 GlobalTransformation = parentTransform * NodeTransformation;

        if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
            uint BoneIndex = m_BoneNameToIndexMap[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
        }

        for (uint i = 0 ; i < node->mNumChildren ; i++) {
            readNodeHierarchy(AnimationTimeTicks, node->mChildren[i], GlobalTransformation);
        }
    }

    
    void getBoneTransforms(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
    {
        glm::mat4 Identity = glm::mat4(1.0f);
        //std::cout << scene->mNumAnimations << std::endl;

        float TicksPerSecond = (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);
        float TimeInTicks = TimeInSeconds * TicksPerSecond;
        float AnimationTimeTicks = fmod(TimeInTicks, (float)scene->mAnimations[0]->mDuration);

        //std::cout << "duration " << (float)scene->mAnimations[0]->mDuration << " ticksPerSecond " << scene->mAnimations[0]->mTicksPerSecond << std::endl;
        //std::cout << "timeInSeconds " << TimeInSeconds << " ticksPerSecond " << TicksPerSecond << " timeInTicks " << TimeInTicks << " AnimationTimeTicks " << AnimationTimeTicks << std::endl;

        readNodeHierarchy(AnimationTimeTicks, scene->mRootNode, Identity);
        Transforms.resize(m_BoneInfo.size());

        for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
            Transforms[i] = m_BoneInfo[i].FinalTransformation;
        }
    }
    

    const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string& nodeName)
    {
        for (uint i = 0 ; i < animation->mNumChannels ; i++) {
            const aiNodeAnim* nodeAnim = animation->mChannels[i];

            if (std::string(nodeAnim->mNodeName.data) == nodeName) {
                return nodeAnim;
            }
        }

        return NULL;
    }

};

#endif

