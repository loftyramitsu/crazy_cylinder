#include "Simulation.h"
#include <iostream>

Simulation::Simulation(int width, int height, const std::string &title)
	: my_width(width), my_height(height), my_title(title), my_window(nullptr) {}

	Simulation::~Simulation() {
		if (my_window)
			glfwDestroyWindow(my_window);
		glfwTerminate();
	}

bool Simulation::initGL(){
	// ---> Initialization of GLFW
	if (!glfwInit()) {
		std::cerr << "Impossible d'initialiser GLFW" << std::endl;
		return false;
	}
	
	// ---> Initialization of the window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	// ---> If the code is compiled on MacOSX
	#ifdef __APPLE__
    		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif
	
	// ---> Creation of the window
	my_window = glfwCreateWindow(my_width, my_height, my_title.c_str(), NULL, NULL);
	if (!my_window){
		std::cerr << "Impossible de créer une fenêtre GLFW" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(my_window);

	// ---> Initialization of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cerr << "Impossible d'initialiser GLAD" << std::endl;
		return false;
	}
	glViewport(0, 0, my_width, my_height);

	return true;
}

void Simulation::render(){
	glfwSwapBuffers(my_window);
}

void Simulation::processInput(){
	
}

void Simulation::run(){
	if (!initGL()) return;

	while (!glfwWindowShouldClose(my_window)){
		processInput();
		render();
		glfwPollEvents();
	}
}


