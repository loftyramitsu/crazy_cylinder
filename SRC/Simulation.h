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

		// Lance la boucle OpenGL et intègre la simulation physique
		// Tmax    : temps physique final
		// U       : vitesse d'entrée (pour condi_lim)
		// eps     : tolérance solveur pression
		// omega   : paramètre SOR
		// maxiter : itérations max solveur pression
		void run(double Tmax, double U, double eps, double omega, int maxiter);

		bool initGL();

	private:
		int my_height, my_width;
		std::string my_title;
		GLFWwindow* my_window;
		Shader* gridShader;
		unsigned int quadVAO, quadVBO;
		unsigned int solidTexture;   // masque solide fixe
		unsigned int dataTexture;    // champ physique mis à jour

		Liquide& fluide;
		std::string champAffiche;    // "ux" | "uy" | "p" | "u_norm"

		void processInput();
		void render();

		std::thread simThread;
		std::atomic<bool> running;
		std::mutex dataMutex;


		// Buffer de rendu séparé (copie légère pour OpenGL)
		std::vector<float> renderBuffer;  // valeurs à afficher
		std::atomic<bool> newFrameReady;  // flag : nouvelle donnée dispo

		void simulationLoop(double Tmax, double U, double eps, double omega, int maxiter);
		void copyFieldToBuffer(); 	// Rempli renderBuffer selon champAffiche

};
