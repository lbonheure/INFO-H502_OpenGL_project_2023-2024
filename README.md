# INFO-H502_OpenGL_project_2023-2024

This is a small project with an animation of a guard that swing its lantern beside a tree against a background of starry sky.

## Author

Luka BONHEURE

## Libraries

A few third party libraries are needed to run the project:

- assimp
- glad
- glfw
- glm
- stb

All are available in the third party directory. Some are included via submodules, so, you must use --recusive flag in git clone to download them.


## How to run

You can use CMake to run the project.


## Controls

There are several commands to navigate in the application. <br/>

**Keyboard (AZERTY):**
- "Z" to move forward (zoom)
- "S" to move backward (de-zoom)
- "Q" to move left
- "D" to move right
- directional keys ("up", "down", "left", "right") to control the rotations of the camera.
- "Escape" to exit
- "Alt" (press and hold) to activate mouse control (and deactivate mouse for rotation)

**Keyboard (QWERTY):**
- "Q" to move forward (zoom)
- "S" to move backward (de-zoom)
- "A" to move left
- "D" to move right
- directional keys ("up", "down", "left", "right") to control the rotations of the camera.
- "Escape" to exit
- "Alt" (press and hold) to activate mouse control (and deactivate mouse for rotation)

**Mouse:**
While Alt key is not pressed, mouse movements control the rotations of the camera.


## Repository organisation

### Objects files

All objects files can be founded in the "objects" directory. <br/>

All files included in the ogldev_guard are retated to an unique object: the guard character.

### Source files

All sources files can be founded in the "project/src" directory.

### CubeMap

The cubeMap files can be founded in the "textures/cubemaps/night" directory.

### Report

You can find the report of this project in the "report" directory.

### video

You can find a small video of the project in the "video" directory.
