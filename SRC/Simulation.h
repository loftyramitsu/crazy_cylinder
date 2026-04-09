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
#include <map>

// Paramètres visuels par champ (gamma + contraste)
struct FieldVisuals {
	float gamma    = 1.0f;
	float contrast = 1.0f;
};

class Simulation {
	public:
		Simulation(int width, int height, const std::string& title,
				Liquide& fluideRef, const std::string& champAffiche);
		~Simulation();

		void run(double Tmax, double U, double eps, int maxiter);
		bool initGL();

	private:
		// --- Fenêtre & rendu ---
		int my_height, my_width;
		std::string my_title;
		GLFWwindow* my_window;
		Shader* gridShader;
		unsigned int quadVAO, quadVBO;
		unsigned int solidTexture;
		unsigned int dataTexture;

		Liquide& fluide;
		std::string champAffiche;

		// Largeur du panneau de contrôle (pixels)
		static constexpr int PANEL_W = 260;

		// --- Thread simulation ---
		std::thread simThread;
		std::atomic<bool> running;
		std::atomic<bool> paused;
		std::mutex dataMutex;
		std::vector<float> renderBuffer;
		std::atomic<bool> newFrameReady;

		void simulationLoop(double Tmax, double eps, int maxiter);
		void copyFieldToBuffer();

		// --- Paramètres de simulation modifiables depuis l'UI ---
		double simU;           // vitesse d'entrée courante
		double simTime = 0.0;

		// --- Enregistrement ---
		bool recording = false;
		double lastCaptureTime = 0.0;
		double captureInterval = 1.0 / 30.0;
		int frameCount = 0;

		// Pipes FFmpeg ouverts par champ
		std::map<std::string, FILE*> ffmpegPipes;
		void openFFmpegPipes();
		void closeFFmpegPipes();
		void captureAllFieldsToPipe();

		// --- Visuel ---
		// Paramètres par champ
		std::map<std::string, FieldVisuals> fieldVisuals;
		void initFieldVisuals();

		// --- UI (rendu manuel GLFW/OpenGL) ---
		// Shader texte-moins : panneau dessiné avec un second shader de rects colorés
		Shader* uiShader;
		unsigned int uiVAO, uiVBO;

		void initUI();
		void renderUI();
		void processInput();
		void render();

		// Espacements blocs
		static constexpr float margin      = 14.0f;
		static constexpr float btnH        = 32.0f;
		static constexpr float sliderH     = 14.0f;

		static constexpr float sectionGap  = 30.0f;
		static constexpr float buttonGap   = 18.0f;
		static constexpr float fieldGap    = 10.0f;

		// Interaction souris
		static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
		static void scrollCallback(GLFWwindow* w, double xoff, double yoff);
		static void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
		void handleClick(double mx, double my);
		void handleScroll(double dy);

		// Générer les vidéos (flush pipes FFmpeg)
		void generateVideos();

		// Helpers UI
		void drawRect(float x, float y, float w, float h, float r, float g, float b, float a);
		void drawText(const std::string& text, float x, float y, float scale);
		bool isInRect(double mx, double my, float x, float y, float w, float h) const;
};
