#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Liquide.h"
#include "Shader.h"
#include <vector>

class Simulation{
	public :
		Simulation(int width, int height, const std::string &title, Liquide& fluideRef);
		~Simulation();

		void run();
		bool initGL();

	private :
		int my_height;
		int my_width;
		std::string my_title;
		GLFWwindow*  my_window;
		Shader* gridShader;
		unsigned int quadVAO, quadVBO;
		unsigned int gridTexture;

		Liquide& fluide;

		void processInput();
		void render();

};
