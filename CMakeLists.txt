cmake_minimum_required(VERSION 3.23)

project(Projet)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---> Copie des shaders dans le dossier de build
file(GLOB SHADERS "SHADERS/*")
file(COPY ${SHADERS}
	DESTINATION ${CMAKE_BINARY_DIR}/SHADERS)
 
# ---> Copie du fichier de configuration dans le dossier de l'exécutable
file(COPY config.ini
	DESTINATION ${CMAKE_BINARY_DIR}/SRC)

# ---> OpenGl needed
find_package(OpenGL)
if(NOT OpenGL_FOUND)
	message(FATAL_ERROR "Il semblerait que OpenGL ne soit pas installé sur cette machine, merci de réessayer après avoir installé une version récente d'OpenGl")
endif()

# ---> Add SRC directory
add_subdirectory(SRC)

# ---> Include GLFW
add_subdirectory(DEP/GLFW)



