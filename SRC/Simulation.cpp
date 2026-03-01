#include "Simulation.h"
#include <iostream>

Simulation::Simulation(int width, int height, const std::string &title, Liquide& fluideRef)
	: my_width(width), my_height(height), my_title(title), my_window(nullptr), fluide(fluideRef) {}

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

	// ---> Load shader
	gridShader = new Shader(
			"SHADERS/grid.vert",
			"SHADERS/grid.frag"
			);

	// Create fullscreen quad
	float quadVertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,	4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,	4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenTextures(1, &gridTexture);
	glBindTexture(GL_TEXTURE_2D, gridTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, fluide.Grid().NX(), fluide.Grid().NY(), 0, GL_RED, GL_UNSIGNED_BYTE, fluide.Grid().SolideTexture().data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return true;
}

void Simulation::render(){
	// ---> Clear Screen
	glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// ---> Render the shader
	gridShader -> use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gridTexture);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(my_window);
}

void Simulation::processInput(){
	if(glfwGetKey(my_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(my_window, true);
}

void Simulation::run(){
	if (!initGL()) return;

	while (!glfwWindowShouldClose(my_window)){
		processInput();
		render();
		glfwPollEvents();
	}
}


