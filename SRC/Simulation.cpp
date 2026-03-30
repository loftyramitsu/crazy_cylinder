#include "Simulation.h"
#include <iostream>
#include <math.h>
#include <algorithm>

Simulation::Simulation(int width, int height, const std::string& title, Liquide& fluideRef, const std::string& champ)
	: my_width(width), my_height(height), my_title(title),
	my_window(nullptr), gridShader(nullptr), fluide(fluideRef),
	champAffiche(champ), solidTexture(0), dataTexture(0) {}

	Simulation::~Simulation() {
		if (gridShader) delete gridShader;
		if (my_window)  glfwDestroyWindow(my_window);
		glfwTerminate();
	}

// Vérifie la compilation d'un shader et affiche l'erreur si besoin
static void checkShaderCompile(unsigned int shader, const std::string& name) {
	int success; char log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, log);
		std::cerr << "[SHADER ERROR] " << name << " : " << log << "\n";
	} else {
		std::cout << "[SHADER OK] " << name << "\n";
	}
}

static void checkProgramLink(unsigned int program) {
	int success; char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::cerr << "[PROGRAM ERROR] Link : " << log << "\n";
	} else {
		std::cout << "[PROGRAM OK] Shader program linked\n";
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// Pour le redimensionnement de la fenêtre -> Sinon l'affichage openGL ne s'adapte pas à la taille de la fenêtre
	glViewport(0, 0, width, height);
}


bool Simulation::initGL() {
	if (!glfwInit()) {
		std::cerr << "Impossible d'initialiser GLFW\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	my_window = glfwCreateWindow(my_width, my_height, my_title.c_str(), NULL, NULL);
	if (!my_window) {
		std::cerr << "Impossible de créer une fenêtre GLFW\n";
		glfwTerminate();
		return false;
	}
	glfwSetWindowAspectRatio(my_window, my_width, my_height);
	glfwMakeContextCurrent(my_window);
	glfwSetFramebufferSizeCallback(my_window, framebuffer_size_callback);

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
	glUniform1i(glGetUniformLocation(gridShader->ID, "uData"),  0);
	glUniform1i(glGetUniformLocation(gridShader->ID, "uSolid"), 1);

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

	const Grille& grille = fluide.Grid();
	int nx = grille.NX(), ny = grille.NY();

	// ---> Texture 1 : masque solide depuis solide[] (masque physique)
	glGenTextures(1, &solidTexture);
	glBindTexture(GL_TEXTURE_2D, solidTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	const std::vector<bool>& solide = grille.Solide();
	std::vector<unsigned char> solidTex(nx * ny);
	int solide_count = 0;
	for (int i = 0; i < nx * ny; i++) {
		solidTex[i] = solide[i] ? 255 : 0;
		if (solide[i]) solide_count++;
	}
	std::cout << "[DEBUG] Cellules solides dans la texture : " << solide_count << " / " << nx*ny << "\n";

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, nx, ny, 0, GL_RED, GL_UNSIGNED_BYTE, solidTex.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// --- Texture 2 : données physiques (mise à jour chaque frame)
	glGenTextures(1, &dataTexture);
	glBindTexture(GL_TEXTURE_2D, dataTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	std::vector<unsigned char> emptyTex(nx * ny, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, nx, ny, 0, GL_RED, GL_UNSIGNED_BYTE, emptyTex.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // smooth pour les données
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::cout << "[INFO] Affichage du champ : " << champAffiche << "\n";



	std::cout << "[DEBUG] solidTexture ID : " << solidTexture << ", dataTexture ID : " << dataTexture << "\n";
	return true;
}

void Simulation::copyFieldToBuffer() {
	// Appelé depuis simulationLoop, sous mutex
	int nx = fluide.Grid().NX();
	int ny = fluide.Grid().NY();
	int n  = nx * ny;

	if (champAffiche == "ux") {
		const auto& tab = fluide.Ux().GetTab();
		renderBuffer.assign(tab.begin(), tab.end());
	} else if (champAffiche == "uy") {
		const auto& tab = fluide.Uy().GetTab();
		renderBuffer.assign(tab.begin(), tab.end());
	} else if (champAffiche == "p") {
		const auto& tab = fluide.P().GetTab();
		renderBuffer.resize(tab.size());

		float mean = 0.f;
		for (float v : tab) mean += v;
		mean /= tab.size();

		for (size_t i = 0; i < tab.size(); i++)
			renderBuffer[i] = (tab[i] - mean)/mean;
		//renderBuffer.assign(tab.begin(), tab.end());
	} else { // u_norm
		const auto& tabX = fluide.Ux().GetTab();
		const auto& tabY = fluide.Uy().GetTab();
		renderBuffer.resize(n);
		for (int i = 0; i < n; i++)
			renderBuffer[i] = (float)std::sqrt(tabX[i]*tabX[i] + tabY[i]*tabY[i]);
	}
}


void Simulation::render() {
	if (newFrameReady.exchange(false)) {
		std::lock_guard<std::mutex> lock(dataMutex);

		int nx = fluide.Grid().NX();
		int ny = fluide.Grid().NY();
		int n  = nx * ny;

		// Normalisation symétrique autour de 0 (ou [0,max] pour u_norm et p)
		float vmax_abs = 0.f;
		for (float v : renderBuffer) vmax_abs = std::max(vmax_abs, std::abs(v));
		if (vmax_abs < 1e-10f) vmax_abs = 1.0f;

		std::vector<unsigned char> tex(n);
		for (int i = 0; i < n; i++) {
			// Mappe [-vmax_abs, +vmax_abs] → [0, 255]
			float normalized = (renderBuffer[i] + vmax_abs) / (2.f * vmax_abs);
			normalized = std::max(0.f, std::min(1.f, normalized));
			tex[i] = (unsigned char)(255.f * normalized);
		}

		glBindTexture(GL_TEXTURE_2D, dataTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny,
				GL_RED, GL_UNSIGNED_BYTE, tex.data());
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	gridShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dataTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, solidTexture);

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
			copyFieldToBuffer();
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
