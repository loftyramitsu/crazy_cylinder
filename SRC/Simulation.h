#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Liquide.h"

class Simulation{
	public :
		Simulation(int width, int height, const std::string &title);
		~Simulation();

		void run();
		bool initGL();

	private :
		int my_height;
		int my_width;
		std::string my_title;
		GLFWwindow*  my_window;

		void processInput();
		void render();

};
