#ifndef OBJECT_ASSIMP_H
#define OBJECT_ASSIMP_H

#include<iostream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include "../shader.h"

using namespace Assimp;

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2

struct Vertex {
	glm::vec3 Position;
	glm::vec2 Texture;
	glm::vec3 Normal;
};


class Object
{
public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Vertex> vertices;


	int numVertices;

	GLuint VBO, VAO;

	glm::mat4 model = glm::mat4(1.0);


	Object(){}
	
	Object(const char* path) {
		std::cout << "load " << path << std::endl;
		Importer importer;

		const aiScene* scene = importer.ReadFile(path, 
								aiProcess_Triangulate  				|
                                aiProcess_GenNormals                |
								aiProcess_JoinIdenticalVertices		|
								aiProcess_ValidateDataStructure);

		//std::cout << "scene imported" << std::endl;

		if (scene == nullptr) {
			std::cout << "Error parsing " << path << ": " << importer.GetErrorString() << std::endl;
  		}

		for (int n=0; n < scene->mNumMeshes; n++){
			const struct aiMesh* mesh = scene->mMeshes[n];
			//std::cout << "mesh" << std::endl;

			//apply_material(scene->mMaterials[mesh->mMaterialIndex]);

			for (int t = 0; t < mesh->mNumFaces; ++t) {
				const struct aiFace* face = &mesh->mFaces[t];
				//std::cout << "face" << std::endl;

				for(int i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
				{
					//std::cout << "vertex" << std::endl;
					Vertex v;
					
					int vertexIndex = face->mIndices[i];	// get group index for current index
					if(mesh->mColors[0] != nullptr)
						//vec4 color = &mesh->mColors[0][vertexIndex];	// color vertex
					if(mesh->mNormals != nullptr)

						if(mesh->HasTextureCoords(0))		//HasTextureCoords(texture_coordinates_set)
						{
							v.Texture = glm::vec2(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
						}

						v.Normal = glm::vec3(mesh->mNormals[vertexIndex].x, mesh->mNormals[vertexIndex].y, mesh->mNormals[vertexIndex].z);
					v.Position = glm::vec3(mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z);

					vertices.push_back(v);
				}
			}
		}
		//std::cout << "Load model with " << vertices.size() << " vertices" << std::endl;
		numVertices = vertices.size();
	}


	void makeObject(Shader shader, bool texture = true) {
		/* This is a working but not perfect solution, you can improve it if you need/want
		* What happens if you call this function twice on an Model ?
		* What happens when a shader doesn't have a position, tex_coord or normal attribute ?
		*/

		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(POSITION_LOCATION);
		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);

		
		if (texture) {
			glEnableVertexAttribArray(TEX_COORD_LOCATION);
			glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			
		}
		
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		
		//desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data;

	}

	void draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	}
};
#endif