#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Liquide.h"
#include "Shader.h"
#include <vector>

#include <thread>
#include <atomic>
#include <mutex>

class Simulation {
	public:
		Simulation(int width, int height, const std::string& title, Liquide& fluideRef, const std::string& champAffiche);
		~Simulation();

		void run(double Tmax, double U, double eps, int maxiter);

		bool initGL();

	private:
		int my_height, my_width;
		std::string my_title;
		GLFWwindow* my_window;
		Shader* gridShader;
		unsigned int quadVAO, quadVBO;
		unsigned int solidTexture;
		unsigned int dataTexture;

		Liquide& fluide;
		std::string champAffiche;

		void processInput();
		void render();

		std::thread simThread;
		std::atomic<bool> running;
		std::mutex dataMutex;

		std::vector<float> renderBuffer;
		std::atomic<bool> newFrameReady;

		void simulationLoop(double Tmax, double U, double eps, int maxiter);
		void copyFieldToBuffer();

		double lastCaptureTime = 0.0;
		double captureInterval = 1.0 / 30.0;
		int frameCount = 0;
		bool recording = true;
		double simTime = 0.0;

		void captureAllFields();
		void generateVideos();
};
