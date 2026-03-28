#include "Simulation.h"
#include <iostream>
#include <math.h>
#include <algorithm>

Simulation::Simulation(int width, int height, const std::string& title, Liquide& fluideRef)
	: my_width(width), my_height(height), my_title(title),
	my_window(nullptr), gridShader(nullptr), fluide(fluideRef) {}

	Simulation::~Simulation() {
		if (gridShader) delete gridShader;
		if (my_window)  glfwDestroyWindow(my_window);
		glfwTerminate();
	}

// Vérifie la compilation d'un shader et affiche l'erreur si besoin
static void checkShaderCompile(unsigned int shader, const std::string& name) {
	int success;
	char log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, log);
		std::cerr << "[SHADER ERROR] " << name << " : " << log << "\n";
	} else {
		std::cout << "[SHADER OK] " << name << "\n";
	}
}

static void checkProgramLink(unsigned int program) {
	int success;
	char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::cerr << "[PROGRAM ERROR] Link : " << log << "\n";
	} else {
		std::cout << "[PROGRAM OK] Shader program linked\n";
	}
}

bool Simulation::initGL() {
	if (!glfwInit()) {
		std::cerr << "Impossible d'initialiser GLFW\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	my_window = glfwCreateWindow(my_width, my_height, my_title.c_str(), NULL, NULL);
	if (!my_window) {
		std::cerr << "Impossible de créer une fenêtre GLFW\n";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(my_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Impossible d'initialiser GLAD\n";
		return false;
	}
	glViewport(0, 0, my_width, my_height);

	// ---> Chargement du shader avec vérification explicite
	gridShader = new Shader("../SHADERS/grid.vert", "../SHADERS/grid.frag");
	checkProgramLink(gridShader->ID);

	// ---> Vérification de l'uniform uGrid
	gridShader->use();
	int uGridLoc = glGetUniformLocation(gridShader->ID, "uGrid");
	std::cout << "[DEBUG] Location uniform 'uGrid' : " << uGridLoc << "\n";
	if (uGridLoc == -1)
		std::cerr << "[WARNING] Uniform 'uGrid' non trouvé dans le shader !\n";
	else
		glUniform1i(uGridLoc, 0);

	// ---> Quad plein écran
	float quadVertices[] = {
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// ---> Texture du masque solide depuis solide[] (masque physique)
	Grille grille = fluide.Grid();
	int nx = grille.NX();
	int ny = grille.NY();
	const std::vector<bool>& solide = grille.Solide();

	std::vector<unsigned char> tex(nx * ny, 0);
	int solide_count = 0;
	for (int i = 0; i < nx * ny; i++) {
		tex[i] = solide[i] ? 255 : 0;
		if (solide[i]) solide_count++;
	}
	std::cout << "[DEBUG] Cellules solides dans la texture : " << solide_count << " / " << nx*ny << "\n";

	glGenTextures(1, &gridTexture);
	glBindTexture(GL_TEXTURE_2D, gridTexture);

	// GL_UNPACK_ALIGNMENT à 1 : important pour les textures dont la largeur
	// n'est pas un multiple de 4 bytes (ici 1 byte par pixel)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, nx, ny, 0, GL_RED, GL_UNSIGNED_BYTE, tex.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Pas de répétition aux bords
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::cout << "[DEBUG] Texture ID : " << gridTexture << "\n";

	return true;
}

void Simulation::render() {
	// Met à jour la texture si nouvelle frame dispo
//	if (newFrameReady.exchange(false)) {
//		std::lock_guard<std::mutex> lock(dataMutex);
//
//		Grille grille = fluide.Grid();
//		int nx = grille.NX();
//		int ny = grille.NY();
//
//		// Normalise uy pour l'affichage
//		float vmax = *std::max_element(renderBuffer.begin(), renderBuffer.end());
//		float vmin = *std::min_element(renderBuffer.begin(), renderBuffer.end());
//		float range = (vmax - vmin) > 1e-10f ? (vmax - vmin) : 1.0f;
//
//		std::vector<unsigned char> tex(nx * ny);
//		for (int i = 0; i < nx * ny; i++) {
//			tex[i] = (unsigned char)(255.f * (renderBuffer[i] - vmin) / range);
//		}
//
//		glBindTexture(GL_TEXTURE_2D, gridTexture);
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny,
//				GL_RED, GL_UNSIGNED_BYTE, tex.data());
//	}
	glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	gridShader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gridTexture);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glfwSwapBuffers(my_window);
}

void Simulation::processInput() {
	if (glfwGetKey(my_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(my_window, true);
}

void Simulation::simulationLoop(double Tmax, double U, double eps, double omega, int maxiter) {
	double T = 0.;
	while (running && T < Tmax) {
		double dt = fluide.CFL();
		// Calculs lourds SANS lock
		fluide.calc_tot_U_star(dt);
		fluide.SolveurPression(eps, dt, omega, maxiter);
		fluide.Contrib(dt);
		fluide.condi_lim(U);
		T += dt;

		// Copie rapide dans le renderBuffer AVEC lock (très court)
		{
			std::lock_guard<std::mutex> lock(dataMutex);
			const auto& tab = fluide.Uy().GetTab();
			renderBuffer.assign(tab.begin(), tab.end());
			newFrameReady = true;
		}

		std::cout << "T = " << T << "  dt = " << dt << "\n";

	}
	running = false;
}

void Simulation::run(double Tmax, double U, double eps, double omega, int maxiter) {
	if (!initGL()) return;

	running = true;
	newFrameReady = false;
	    renderBuffer.resize(fluide.Grid().NX() * fluide.Grid().NY(), 0.f);
	// Lance la simulation dans un thread séparé
	simThread = std::thread(&Simulation::simulationLoop, this,
			Tmax, U, eps, omega, maxiter);

	// Le thread principal gère uniquement OpenGL
	while (!glfwWindowShouldClose(my_window)) {
		processInput();
		render();
		glfwPollEvents();
	}

	// Arrêt propre
	running = false;
	if (simThread.joinable()) simThread.join();


}
