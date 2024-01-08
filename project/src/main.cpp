#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>

#include "camera.h"
#include "shader.h"
#include "cubeMap.h"
#include "utils/utils.h"

#include "meshes/object.h"
#include "meshes/static_object.h"
#include "meshes/animated_object.h"

#include "light.h"


#define HALF_PI 1.57079632679489661923132169163975144f


#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif


void setMaterial(const Material& material, Shader shader)
{
	shader.setVector3f("gMaterial.AmbientColor", material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
	shader.setVector3f("gMaterial.DiffuseColor", material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
	shader.setVector3f("gMaterial.SpecularColor", material.SpecularColor.r, material.SpecularColor.g, material.SpecularColor.b);
}

void setCameraLocalPos(glm::vec3 CameraLocalPos3f, Shader shader)
{
	shader.setVector3f("gCameraLocalPos", CameraLocalPos3f);
}


void init_OpenGL()
{
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}


int main(int argc, char* argv[])
{
	std::cout << "Welcome in my OpenGL project!" << std::endl;

	//Boilerplate
	init_OpenGL();

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}
	glfwMakeContextCurrent(window);

	// For screen resolution
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// For camera rotation with mouse
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}
	glEnable(GL_DEPTH_TEST);

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	// For screen resolution
	int framebuffer_width, framebuffer_height;

	/******************
	* Include Shaders *
	*******************/
	const char sourceV_character[] = PATH_TO_PROJECT_SHADERS "/vertex_skinning.cpp";
	const char sourceF_character[] = PATH_TO_PROJECT_SHADERS "/fragment_skinning.cpp";

	Shader shader_character(sourceV_character, sourceF_character);

	const char sourceV_ground[] = PATH_TO_PROJECT_SHADERS "/vertex_ground.cpp";
	const char sourceF_ground[] = PATH_TO_PROJECT_SHADERS "/fragment_ground.cpp";

	Shader shader_ground(sourceV_ground, sourceF_ground);
	
	const char sourceV_tree[] = PATH_TO_PROJECT_SHADERS "/vertex_tree.cpp";
	const char sourceF_tree[] = PATH_TO_PROJECT_SHADERS "/fragment_tree.cpp";

	Shader shader_tree(sourceV_tree, sourceF_tree);
	

	/******************
	* Include Objects *
	*******************/

	char path_character[] = PATH_TO_OBJECTS "/ogldev_guard/boblampclean.md5mesh";//"/man/model.dae"; //"/simple/model.dae";//"/ogldev_ex/boblampclean.md5mesh";//"/mc_walking/mc_walking.dae";
	AnimatedObject character = AnimatedObject();
	character.LoadMesh(path_character);

	char path_ground[] = PATH_TO_OBJECTS "/plane.obj";
	Object ground = Object(path_ground);
	ground.makeObject(shader_ground, false);

	char path_tree[] = PATH_TO_OBJECTS "/sapin.dae";
	StaticObject tree = StaticObject();
	tree.LoadMesh(path_tree);

	/**********
	* CubeMap *
	***********/
	CubeMap cubeMap = CubeMap();
	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/night/";
	cubeMap.loadTexture(pathToCubeMap);

	/*****************
	* Transformation *
	******************/
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	// init model ground
	glm::mat4 modelGround = glm::mat4(1.0);
	modelGround = glm::scale(modelGround, glm::vec3(20, 1, 20));
	modelGround = glm::translate(modelGround, glm::vec3(0,-4.5,0));

	// init model tree
	glm::mat4 modelTree = glm::mat4(1.0);
	modelTree = glm::scale(modelTree, glm::vec3(3, 2.5, 3));
	modelTree = glm::translate(modelTree, glm::vec3(1.5,-0.18,-1.5));
	modelTree = glm::rotate(modelTree, -HALF_PI, glm::vec3(1,0,0));

	// Init texture
	shader_character.use();
	shader_character.setInteger("gSampler", COLOR_TEXTURE_UNIT_INDEX);
	shader_character.setInteger("gSamplerSpecularExponent", SPECULAR_EXPONENT_UNIT_INDEX);

	shader_tree.use();
	shader_tree.setInteger("gSampler", COLOR_TEXTURE_UNIT_INDEX);
	shader_tree.setInteger("gSamplerSpecularExponent", SPECULAR_EXPONENT_UNIT_INDEX);

	// Init worldTransform
	WorldTrans& worldTransform = character.getWorldTransform();
	worldTransform.SetRotation(90.0f, 180.0f, 180.0f);
	worldTransform.SetPosition(-15.0f, 50.0f, -45.0f);
    worldTransform.SetScale(0.1f);
	glm::mat4 World = worldTransform.GetMatrix();

	// Init Lighting
	Lighting lighting = Lighting();
	lighting.init();

	shader_character.use();
	lighting.render(shader_character, worldTransform, camera.Position, camera.Front);

	shader_tree.use();
	lighting.render(shader_tree, worldTransform, camera.Position, camera.Front);


	glfwSwapInterval(1);
	//Rendering
	auto lastFrameTime = glfwGetTime();
	auto starting_t = lastFrameTime;
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		
		glfwPollEvents();
		
		double now = glfwGetTime();

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view = camera.GetViewMatrix();
		// For screen resolution
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		double ratio = framebuffer_width/ framebuffer_height;
		perspective = camera.GetProjectionMatrix(45.0, ratio);

		// Use the shader Class to send the uniform
		shader_character.use();
		lighting.render(shader_character, worldTransform, camera.Position, camera.Front);

		setMaterial(character.getMaterial(), shader_character);
		glm::vec3 CameraLocalPos3f = worldTransform.WorldPosToLocalPos(camera.Position);
		setCameraLocalPos(CameraLocalPos3f, shader_character);

		float AnimationTimeSec = (float)(now - starting_t);
		
		std::vector<glm::mat4> transforms;
		character.getBoneTransforms(AnimationTimeSec, transforms);
		shader_character.setMatrix4Array("gBones", transforms, transforms.size());
		shader_character.setMatrix4("M", World);
		shader_character.setMatrix4("V", view);
		shader_character.setMatrix4("P", perspective);

		glDepthFunc(GL_LEQUAL);
		character.render();

		shader_ground.use();
		shader_ground.setMatrix4("M", modelGround);
		shader_ground.setMatrix4("V", view);
		shader_ground.setMatrix4("P", perspective);

		ground.draw();

		shader_tree.use();
		lighting.render(shader_tree, worldTransform, camera.Position, camera.Front);
		setMaterial(tree.getMaterial(), shader_tree);
		setCameraLocalPos(CameraLocalPos3f, shader_tree);
		shader_tree.setMatrix4("M", modelTree);
		shader_tree.setMatrix4("V", view);
		shader_tree.setMatrix4("P", perspective);

		tree.render();

		// CubeMap rendering
		cubeMap.render(view, perspective);

		fps(now);
		lastFrameTime = now;
		
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	std::cout << std::endl;

	return 0;
}