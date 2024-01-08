#for INFOH502 course
#Heavily inspired from https://github.com/Polytonic/Glitter/blob/master/CMakeLists.txt
cmake_minimum_required (VERSION 3.21)
project(opengl_project_info-h502)


set(COMPILE_SIMPLE_PROJECT TRUE CACHE BOOL "compile project")

set(CMAKE_CXX_STANDARD 17)


find_package(OpenGL REQUIRED)

#for glad library
add_library( glad STATIC 3rdParty/glad/src/glad.c "project/src/main.cpp")
set(GLAD_INCLUDE "3rdParty/glad/include")
target_include_directories(glad PUBLIC ${GLAD_INCLUDE})

#for GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)

#Search for glfw in case it is installed
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
    #if glfw is not found, we will use the sources from the submodules
    add_subdirectory(3rdParty/glfw)
    include_directories(3rdParty/glfw/include/)
endif()


find_package(Bullet)
if(NOT Bullet_FOUND)
    option(BUILD_BULLET2_DEMOS OFF)
    option(BUILD_CPU_DEMOS OFF)
    option(BUILD_EXTRAS OFF)
    option(BUILD_OPENGL3_DEMOS OFF)
    option(BUILD_UNIT_TESTS OFF)
    add_subdirectory(3rdParty/bullet)
    include_directories(3rdParty/bullet/src)
else()
    include_directories(${BULLET_INCLUDE_DIR}) 
endif()

include_directories(3rdParty/glad/include/
                    3rdParty/glm/
                    3rdParty/stb/)

find_package(assimp)
if(NOT assimp_FOUND)
    add_subdirectory(3rdParty/assimp)
    include_directories(3rdParty/assimp/include/)
else()
    include_directories(${ASSIMP_INCLUDE_DIR})
endif()


add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/textures")
add_compile_definitions(PATH_TO_PROJECT_SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/project/src/shaders")

if(COMPILE_SIMPLE_PROJECT)
	add_subdirectory(project)
endif()