cmake_minimum_required (VERSION 3.21)


set(SRC_PROJECT
"src/main.cpp" 
"src/camera.h"
"src/shader.h" 
"src/cubeMap.h"
"src/utils/utils.h")

add_executable(${PROJECT_NAME}_project ${SRC_PROJECT} )

target_include_directories(${PROJECT_NAME}_project PUBLIC ${GLAD_INCLUDE} ) 
target_link_libraries(${PROJECT_NAME}_project PUBLIC glad OpenGL::GL glfw LinearMath assimp)