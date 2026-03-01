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
	return false;
}

void Simulation::render(){}

void Simulation::run(){}


