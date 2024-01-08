cmake_minimum_required (VERSION 3.21)


set(SRC_SIMPLE_PROJECT
"src/main.cpp" 
"src/camera.h"
"src/shader.h" 
"src/cubeMap.h"
"src/utils/utils.h")

add_executable(${PROJECT_NAME}_simple_project ${SRC_SIMPLE_PROJECT} )

target_include_directories(${PROJECT_NAME}_simple_project PUBLIC ${GLAD_INCLUDE} ) 
target_link_libraries(${PROJECT_NAME}_simple_project PUBLIC glad OpenGL::GL glfw LinearMath assimp)