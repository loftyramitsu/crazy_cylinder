#include "Simulation.h"
#include <iostream>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void checkProgramLink(unsigned int program) {
	int success; char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::cerr << "[PROGRAM ERROR] " << log << "\n";
	}
}

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
	glViewport(0, 0, w, h);
}

// ─── Listes de champs ─────────────────────────────────────────────────────────
static const std::vector<std::string> FIELD_NAMES = {"ux", "uy", "p", "u_norm", "vort"};

// ─── Constructeur / Destructeur ───────────────────────────────────────────────

Simulation::Simulation(int width, int height, const std::string& title,
		Liquide& fluideRef, const std::string& champ)
	: my_width(width), my_height(height), my_title(title),
	my_window(nullptr), gridShader(nullptr), uiShader(nullptr),
	fluide(fluideRef), champAffiche(champ),
	solidTexture(0), dataTexture(0),
	running(false), paused(false), newFrameReady(false),
	simU(1.0)
{
	initFieldVisuals();
}

Simulation::~Simulation() {
	closeFFmpegPipes();
	if (gridShader) delete gridShader;
	if (uiShader)   delete uiShader;
	if (my_window)  glfwDestroyWindow(my_window);
	glfwTerminate();
}

// ─── Valeurs gamma/contraste par défaut par champ ─────────────────────────────
void Simulation::initFieldVisuals() {
	// gamma < 1 booste les faibles valeurs (plus lumineux pour champs diffus)
	fieldVisuals["ux"]     = {0.8f, 1.2f};
	fieldVisuals["uy"]     = {0.8f, 1.2f};
	fieldVisuals["p"]      = {0.45f, 2.5f};  // pression : très contrasté + lumineux
	fieldVisuals["u_norm"] = {1.0f, 1.0f};
	fieldVisuals["vort"]   = {0.4f, 3.0f};   // vorticité : fort boost
}

// ─── Initialisation OpenGL ────────────────────────────────────────────────────

bool Simulation::initGL() {
	if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return false; }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	my_window = glfwCreateWindow(my_width + PANEL_W, my_height, my_title.c_str(), NULL, NULL);
	if (!my_window) { std::cerr << "Window creation failed\n"; glfwTerminate(); return false; }

	glfwMakeContextCurrent(my_window);
	glfwSetFramebufferSizeCallback(my_window, framebuffer_size_callback);
	glfwSetWindowUserPointer(my_window, this);
	glfwSetMouseButtonCallback(my_window, mouseButtonCallback);
	glfwSetScrollCallback(my_window, scrollCallback);
	glfwSetKeyCallback(my_window, keyCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "GLAD init failed\n"; return false;
	}
	glViewport(0, 0, my_width + PANEL_W, my_height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// --- Shader grille ---
	gridShader = new Shader("../SHADERS/grid.vert", "../SHADERS/grid.frag");
	checkProgramLink(gridShader->ID);
	gridShader->use();
	glUniform1i(glGetUniformLocation(gridShader->ID, "uData"),  0);
	glUniform1i(glGetUniformLocation(gridShader->ID, "uSolid"), 1);

	// --- Quad qui couvre la zone simulation (à gauche) ---
	// Coordonnées normalisées : la zone simulation va de -1 à (my_width/(my_width+PANEL_W))*2-1
	float simRight = (float)my_width / (float)(my_width + PANEL_W) * 2.0f - 1.0f;
	float quadVertices[] = {
		-1.0f,    -1.0f,    0.0f, 0.0f,
		simRight, -1.0f,    1.0f, 0.0f,
		simRight,  1.0f,    1.0f, 1.0f,
		-1.0f,    -1.0f,    0.0f, 0.0f,
		simRight,  1.0f,    1.0f, 1.0f,
		-1.0f,     1.0f,    0.0f, 1.0f
	};
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);

	// --- Textures ---
	const Grille& grille = fluide.Grid();
	int nx = grille.NX(), ny = grille.NY();
	const std::vector<bool>& solide = grille.Solide();
	std::vector<unsigned char> solidTex(nx * ny);
	for (int i = 0; i < nx * ny; i++) solidTex[i] = solide[i] ? 255 : 0;

	glGenTextures(1, &solidTexture);
	glBindTexture(GL_TEXTURE_2D, solidTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, nx, ny, 0, GL_RED, GL_UNSIGNED_BYTE, solidTex.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &dataTexture);
	glBindTexture(GL_TEXTURE_2D, dataTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	std::vector<unsigned char> emptyTex(nx * ny, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, nx, ny, 0, GL_RED, GL_UNSIGNED_BYTE, emptyTex.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	initUI();
	return true;
}

// ─── Shader UI minimaliste (rects colorés) ────────────────────────────────────
// On dessine le panneau avec un shader très simple qui reçoit
// position + couleur en attributs.

static const char* UI_VERT = R"(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec4 aCol;
out vec4 vCol;
void main(){ gl_Position = vec4(aPos, 0.0, 1.0); vCol = aCol; }
)";
static const char* UI_FRAG = R"(
#version 330 core
in vec4 vCol;
out vec4 FragColor;
void main(){ FragColor = vCol; }
)";

void Simulation::initUI() {
	// Compiler shader UI inline
	auto compile = [](const char* src, GLenum type) -> unsigned int {
		unsigned int s = glCreateShader(type);
		glShaderSource(s, 1, &src, nullptr);
		glCompileShader(s);
		return s;
	};
	unsigned int vs = compile(UI_VERT, GL_VERTEX_SHADER);
	unsigned int fs = compile(UI_FRAG, GL_FRAGMENT_SHADER);
	unsigned int prog = glCreateProgram();
	glAttachShader(prog, vs); glAttachShader(prog, fs);
	glLinkProgram(prog);
	glDeleteShader(vs); glDeleteShader(fs);

	// On stocke l'ID dans uiShader->ID via un Shader factice
	// Pour éviter de dépendre de Shader, on crée une classe locale.
	// Ici on utilise directement un unsigned int stocké via un Shader vide.
	// Solution simple : on réutilise le même Shader mais on stocke l'ID manuellement.
	uiShader = new Shader(); // Shader par défaut (constructeur vide, ID=prog)
	uiShader->ID = prog;

	glGenVertexArrays(1, &uiVAO);
	glGenBuffers(1, &uiVBO);
	glBindVertexArray(uiVAO);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
	// Buffer dynamique — on le remplit à chaque frame
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*6*4096, nullptr, GL_DYNAMIC_DRAW);
	// aPos (vec2) + aCol (vec4) = 6 floats par vertex
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

// Convertit pixel (0..totalWidth, 0..height) en NDC
static float px2ndcX(float px, int totalW) { return  px / totalW * 2.0f - 1.0f; }
static float px2ndcY(float py, int totalH) { return  py / totalH * 2.0f - 1.0f; }

// Ajoute un rect (6 vertices) dans le buffer
static void pushRect(std::vector<float>& buf,
		float x0, float y0, float x1, float y1,
		float r, float g, float b, float a)
{
	// Triangle 1
	buf.insert(buf.end(), {x0,y0, r,g,b,a,  x1,y0, r,g,b,a,  x1,y1, r,g,b,a});
	// Triangle 2
	buf.insert(buf.end(), {x0,y0, r,g,b,a,  x1,y1, r,g,b,a,  x0,y1, r,g,b,a});
}

// ─── Mini bitmap-font 5×7 (ASCII 32-90) ──────────────────────────────────────
// Chaque caractère = 5 colonnes de 7 bits (bit0=haut)
static const uint8_t FONT5x7[][5] = {
	{0x00,0x00,0x00,0x00,0x00}, // ' '
	{0x00,0x00,0x5F,0x00,0x00}, // '!'
	{0x00,0x07,0x00,0x07,0x00}, // '"'
	{0x14,0x7F,0x14,0x7F,0x14}, // '#'
	{0x24,0x2A,0x7F,0x2A,0x12}, // '$'
	{0x23,0x13,0x08,0x64,0x62}, // '%'
	{0x36,0x49,0x55,0x22,0x50}, // '&'
	{0x00,0x05,0x03,0x00,0x00}, // '\''
	{0x00,0x1C,0x22,0x41,0x00}, // '('
	{0x00,0x41,0x22,0x1C,0x00}, // ')'
	{0x08,0x2A,0x1C,0x2A,0x08}, // '*'
	{0x08,0x08,0x3E,0x08,0x08}, // '+'
	{0x00,0x50,0x30,0x00,0x00}, // ','
	{0x08,0x08,0x08,0x08,0x08}, // '-'
	{0x00,0x60,0x60,0x00,0x00}, // '.'
	{0x20,0x10,0x08,0x04,0x02}, // '/'
	{0x3E,0x51,0x49,0x45,0x3E}, // '0'
	{0x00,0x42,0x7F,0x40,0x00}, // '1'
	{0x42,0x61,0x51,0x49,0x46}, // '2'
	{0x21,0x41,0x45,0x4B,0x31}, // '3'
	{0x18,0x14,0x12,0x7F,0x10}, // '4'
	{0x27,0x45,0x45,0x45,0x39}, // '5'
	{0x3C,0x4A,0x49,0x49,0x30}, // '6'
	{0x01,0x71,0x09,0x05,0x03}, // '7'
	{0x36,0x49,0x49,0x49,0x36}, // '8'
	{0x06,0x49,0x49,0x29,0x1E}, // '9'
	{0x00,0x36,0x36,0x00,0x00}, // ':'
	{0x00,0x56,0x36,0x00,0x00}, // ';'
	{0x00,0x08,0x14,0x22,0x41}, // '<'
	{0x14,0x14,0x14,0x14,0x14}, // '='
	{0x41,0x22,0x14,0x08,0x00}, // '>'
	{0x02,0x01,0x51,0x09,0x06}, // '?'
	{0x32,0x49,0x79,0x41,0x3E}, // '@'
	{0x7E,0x11,0x11,0x11,0x7E}, // 'A'
	{0x7F,0x49,0x49,0x49,0x36}, // 'B'
	{0x3E,0x41,0x41,0x41,0x22}, // 'C'
	{0x7F,0x41,0x41,0x22,0x1C}, // 'D'
	{0x7F,0x49,0x49,0x49,0x41}, // 'E'
	{0x7F,0x09,0x09,0x09,0x01}, // 'F'
	{0x3E,0x41,0x49,0x49,0x7A}, // 'G'
	{0x7F,0x08,0x08,0x08,0x7F}, // 'H'
	{0x00,0x41,0x7F,0x41,0x00}, // 'I'
	{0x20,0x40,0x41,0x3F,0x01}, // 'J'
	{0x7F,0x08,0x14,0x22,0x41}, // 'K'
	{0x7F,0x40,0x40,0x40,0x40}, // 'L'
	{0x7F,0x02,0x04,0x02,0x7F}, // 'M'
	{0x7F,0x04,0x08,0x10,0x7F}, // 'N'
	{0x3E,0x41,0x41,0x41,0x3E}, // 'O'
	{0x7F,0x09,0x09,0x09,0x06}, // 'P'
	{0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
	{0x7F,0x09,0x19,0x29,0x46}, // 'R'
	{0x46,0x49,0x49,0x49,0x31}, // 'S'
	{0x01,0x01,0x7F,0x01,0x01}, // 'T'
	{0x3F,0x40,0x40,0x40,0x3F}, // 'U'
	{0x1F,0x20,0x40,0x20,0x1F}, // 'V'
	{0x3F,0x40,0x38,0x40,0x3F}, // 'W'
	{0x63,0x14,0x08,0x14,0x63}, // 'X'
	{0x03,0x04,0x78,0x04,0x03}, // 'Y'
	{0x61,0x51,0x49,0x45,0x43}, // 'Z'
};

// Dessine une chaîne (convertie en majuscules) sous forme de micro-rects dans buf.
// px, py = coin haut-gauche en pixels (y depuis le haut de la fenêtre).
// scale  = taille d'un pixel de la font en pixels écran.
static void pushText(std::vector<float>& buf,
		const std::string& text,
		float px, float py,
		float scale,
		int fw, int fh,
		float r, float g, float b)
{
	float cx = px;
	for (char raw : text) {
		char c = (char)toupper((unsigned char)raw);
		int idx = (int)c - 32;
		if (idx < 0 || idx >= (int)(sizeof(FONT5x7)/sizeof(FONT5x7[0]))) {
			cx += (5 + 1) * scale;
			continue;
		}
		const uint8_t* col = FONT5x7[idx];
		for (int col_i = 0; col_i < 5; col_i++) {
			for (int row_i = 0; row_i < 7; row_i++) {
				if (col[col_i] & (1 << row_i)) {
					float x0 = cx + col_i * scale;
					float y0 = py + row_i * scale;   // y depuis le haut
					float x1 = x0 + scale;
					float y1 = y0 + scale;
					// Conversion en NDC (y flippé)
					float nx0 = x0 / fw * 2.f - 1.f;
					float ny0 = 1.f - y1 / fh * 2.f;
					float nx1 = x1 / fw * 2.f - 1.f;
					float ny1 = 1.f - y0 / fh * 2.f;
					buf.insert(buf.end(), {
							nx0,ny0, r,g,b,1.f,  nx1,ny0, r,g,b,1.f,  nx1,ny1, r,g,b,1.f,
							nx0,ny0, r,g,b,1.f,  nx1,ny1, r,g,b,1.f,  nx0,ny1, r,g,b,1.f
							});
				}
			}
		}
		cx += (5 + 1) * scale;
	}
}

// ─── Rendu UI ─────────────────────────────────────────────────────────────────

void Simulation::renderUI() {
	int fw, fh;
	glfwGetFramebufferSize(my_window, &fw, &fh);
	float totalW = (float)fw, totalH = (float)fh;

	// Zone panneau (pixels) : x de my_width..my_width+PANEL_W
	float px0 = (float)my_width, px1 = (float)(my_width + PANEL_W);

	std::vector<float> buf;
	buf.reserve(6*6*64);

	auto N = [&](float px, float py) -> std::pair<float,float> {
		return { px2ndcX(px, fw), px2ndcY(py, fh) };
	};

	// Fond panneau
	{auto [ax,ay]=N(px0,0); auto [bx,by]=N(px1,totalH);
		pushRect(buf, ax, ay, bx, by, 0.13f,0.13f,0.15f, 1.0f);}

	// Séparateur gauche
	{auto [ax,ay]=N(px0,0); auto [bx,by]=N(px0+1,totalH);
		pushRect(buf, ax, ay, bx, by, 0.35f,0.35f,0.40f, 1.0f);}

	float curY    = totalH - 20.0f; // on part du haut (pixels)

	bool locked = recording; // quand on enregistre, tout est verrouillé sauf Stop

	// ── Titre panneau ──
	curY -= sectionGap;
	// (texte rendu via la console, pas de font rendering ici — on dessine un trait décoratif)
	{auto [ax,ay]=N(px0+margin, curY-2); auto [bx,by]=N(px1-margin, curY);
		pushRect(buf, ax, ay, bx, by, 0.50f,0.55f,0.65f, 0.9f);}
	curY -= sectionGap;
	// Label titre panneau
	pushText(buf, "CONTROLES", px0 + margin, totalH - (curY + 10.f), 1.5f, fw, fh, 0.70f,0.75f,0.85f);

	// ══════════════════════════════════════════════════════════════
	// 1. SÉLECTEUR DE CHAMP
	// ══════════════════════════════════════════════════════════════
	curY -= sectionGap;
	// Label section
	pushText(buf, "CHAMP AFFICHE", px0 + margin, totalH - curY - 11.f, 1.2f, fw, fh, 0.55f,0.60f,0.70f);
	for (int fi = 0; fi < (int)FIELD_NAMES.size(); fi++) {
		bool active  = (FIELD_NAMES[fi] == champAffiche);
		bool enabled = !locked;
		float r = active ? 0.30f : (enabled ? 0.20f : 0.15f);
		float g = active ? 0.55f : (enabled ? 0.20f : 0.15f);
		float b = active ? 0.80f : (enabled ? 0.22f : 0.17f);
		float btnW = (px1 - px0 - 2.0f*margin - 3.0f*4.0f) / 5.0f; // 5 boutons sur une ligne
		float bx0 = px0 + margin + fi*(btnW+4.0f);
		float bx1 = bx0 + btnW;
		{auto [ax,ay]=N(bx0, curY-btnH+4); auto [cx,cy]=N(bx1, curY);
			pushRect(buf, ax, ay, cx, cy, r,g,b, enabled||active ? 1.0f : 0.5f);}
	}
	// Labels des boutons champ
	{
		const char* labels[] = {"UX","UY","P","NORM","VORT"};
		for (int fi = 0; fi < 5; fi++) {
			float btnW2 = (px1 - px0 - 2.f*margin - 3.f*4.f) / 5.f;
			float bx0 = px0 + margin + fi*(btnW2+4.f);
			float lx = bx0 + 3.f;
			float ly = totalH - curY + 6.f;
			bool active = (FIELD_NAMES[fi] == champAffiche);
			float lr = active ? 1.f : 0.65f;
			float lg = active ? 1.f : 0.65f;
			float lb = active ? 1.f : 0.65f;
			pushText(buf, labels[fi], lx, ly, 1.2f, fw, fh, lr, lg, lb);
		}
	}
	curY -= btnH + sectionGap;

	// ══════════════════════════════════════════════════════════════
	// 2. GAMMA (slider)
	// ══════════════════════════════════════════════════════════════
	float sliderX0 = px0 + margin, sliderX1 = px1 - margin;
	float sliderW = sliderX1 - sliderX0;
	// Label gamma avec valeur
	{
		float gv = fieldVisuals[champAffiche].gamma;
		std::ostringstream lbl; lbl << "GAMMA  " << std::fixed << std::setprecision(2) << gv;
		pushText(buf, lbl.str(), sliderX0, totalH - curY - 11.f, 1.2f, fw, fh, 0.55f,0.70f,0.90f);
	}
	// Track
	{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX1, curY);
		pushRect(buf, ax, ay, bx, by, 0.25f,0.25f,0.28f, 1.0f);}
	// Fill gamma
	float gVal = fieldVisuals[champAffiche].gamma;
	float gNorm = std::max(0.f, std::min(1.f, (gVal - 0.1f) / 1.9f)); // [0.1..2.0] -> [0..1]
	{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX0+gNorm*sliderW, curY);
		float col = locked ? 0.2f : 0.4f;
		pushRect(buf, ax, ay, bx, by, col, col+0.2f, col+0.4f, 1.0f);}
	// Thumb
	{float tx=sliderX0+gNorm*sliderW;
		auto [ax,ay]=N(tx-4, curY-sliderH-3); auto [bx,by]=N(tx+4, curY+3);
		pushRect(buf, ax, ay, bx, by, 0.85f,0.85f,0.90f, locked?0.3f:1.0f);}
	curY -= sliderH + sectionGap;

	// ══════════════════════════════════════════════════════════════
	// 3. CONTRASTE (slider)
	// ══════════════════════════════════════════════════════════════
	// Label contraste avec valeur
	{
		float cv = fieldVisuals[champAffiche].contrast;
		std::ostringstream lbl; lbl << "CONTRASTE  " << std::fixed << std::setprecision(2) << cv;
		pushText(buf, lbl.str(), sliderX0, totalH - curY - 11.f, 1.2f, fw, fh, 0.90f,0.60f,0.50f);
	}
	{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX1, curY);
		pushRect(buf, ax, ay, bx, by, 0.25f,0.25f,0.28f, 1.0f);}
	float cVal  = fieldVisuals[champAffiche].contrast;
	float cNorm = std::max(0.f, std::min(1.f, (cVal - 0.5f) / 3.5f)); // [0.5..4.0] -> [0..1]
	{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX0+cNorm*sliderW, curY);
		float col = locked ? 0.2f : 0.55f;
		pushRect(buf, ax, ay, bx, by, col+0.15f, col, col, 1.0f);}
	{float tx=sliderX0+cNorm*sliderW;
		auto [ax,ay]=N(tx-4, curY-sliderH-3); auto [bx,by]=N(tx+4, curY+3);
		pushRect(buf, ax, ay, bx, by, 0.85f,0.85f,0.90f, locked?0.3f:1.0f);}
	curY -= sliderH + sectionGap;

	// ══════════════════════════════════════════════════════════════
	// 4. START / PAUSE / STOP
	// ══════════════════════════════════════════════════════════════
	// Label section
	pushText(buf, "SIMULATION", sliderX0, totalH - curY - 11.f, 1.2f, fw, fh, 0.55f,0.60f,0.70f);
	float halfW = (sliderW - buttonGap) / 2.0f;
	// Bouton START/PAUSE
	{bool isPaused = paused.load();
		float r = isPaused ? 0.20f : 0.10f;
		float g = isPaused ? 0.55f : 0.42f;
		float b = isPaused ? 0.20f : 0.10f;
		auto [ax,ay]=N(sliderX0, curY-btnH); auto [bx,by]=N(sliderX0+halfW, curY);
		pushRect(buf, ax, ay, bx, by, r,g,b, 1.0f);
		// Texte sur le bouton
		pushText(buf, isPaused ? "REPRENDRE" : "PAUSE",
				sliderX0 + 4.f, totalH - curY + (btnH * 0.5f) - 5.f,
				1.2f, fw, fh, 1.f, 1.f, 1.f);}
		// Bouton STOP
		{auto [ax,ay]=N(sliderX0+halfW+buttonGap, curY-btnH); auto [bx,by]=N(sliderX1, curY);
			pushRect(buf, ax, ay, bx, by, 0.55f,0.10f,0.10f, 1.0f);
			pushText(buf, "STOP",
					sliderX0 + halfW + buttonGap + 10.f, totalH - curY + (btnH * 0.5f) - 5.f,
					1.2f, fw, fh, 1.f, 0.7f, 0.7f);}
			curY -= btnH + sectionGap;

			// ══════════════════════════════════════════════════════════════
			// 5. VITESSE U (slider)
			// ══════════════════════════════════════════════════════════════
			// Label vitesse avec valeur
			{
				std::ostringstream lbl; lbl << "VITESSE U  " << std::fixed << std::setprecision(2) << simU;
				pushText(buf, lbl.str(), sliderX0, totalH - curY - 11.f, 1.2f, fw, fh, 0.60f,0.85f,0.55f);
			}
			{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX1, curY);
				pushRect(buf, ax, ay, bx, by, 0.25f,0.25f,0.28f, 1.0f);}
			float uNorm = std::max(0.f, std::min(1.f, (float)(simU - 0.1) / 4.9f)); // [0.1..5.0] -> [0..1]
			{auto [ax,ay]=N(sliderX0, curY-sliderH); auto [bx,by]=N(sliderX0+uNorm*sliderW, curY);
				float col = locked ? 0.15f : 0.45f;
				pushRect(buf, ax, ay, bx, by, col+0.15f, col+0.25f, col, 1.0f);}
			{float tx=sliderX0+uNorm*sliderW;
				auto [ax,ay]=N(tx-4, curY-sliderH-3); auto [bx,by]=N(tx+4, curY+3);
				pushRect(buf, ax, ay, bx, by, 0.85f,0.85f,0.90f, locked?0.3f:1.0f);}
			curY -= sliderH + sectionGap;

			// ══════════════════════════════════════════════════════════════
			// 6. BOUTON ENREGISTREMENT
			// ══════════════════════════════════════════════════════════════
			// Label section
			pushText(buf, "ENREGISTREMENT", sliderX0, totalH - curY - 11.f, 1.2f, fw, fh, 0.55f,0.60f,0.70f);
			{float r = recording ? 0.65f : 0.20f;
				float g = recording ? 0.12f : 0.20f;
				float b = recording ? 0.12f : 0.20f;
				auto [ax,ay]=N(sliderX0, curY-btnH); auto [bx,by]=N(sliderX1, curY);
				pushRect(buf, ax, ay, bx, by, r,g,b, 1.0f);}
			// Indicateur LED
			{float ledX = sliderX0 + 14.0f, ledY = curY - btnH/2.0f;
				auto [ax,ay]=N(ledX-6, ledY-6); auto [bx,by]=N(ledX+6, ledY+6);
				pushRect(buf, ax, ay, bx, by,
						recording ? 1.0f : 0.3f,
						recording ? 0.1f : 0.3f,
						recording ? 0.1f : 0.3f, 1.0f);}
				// Texte sur le bouton
				{
					float ledX2 = sliderX0 + 14.f;
					pushText(buf, recording ? "  STOP REC" : "  START REC",
							ledX2 + 8.f, totalH - curY + (btnH * 0.5f) - 5.f,
							1.2f, fw, fh, 1.f, recording ? 0.5f : 1.f, recording ? 0.5f : 1.f);
				}

				// Upload et draw
				glUseProgram(uiShader->ID);
				glBindVertexArray(uiVAO);
				glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, buf.size()*sizeof(float), buf.data());
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(buf.size()/6));
				glBindVertexArray(0);

				// ── Labels console (fallback : on affiche dans le titre de la fenêtre) ──
				{
					std::ostringstream oss;
					oss << my_title
						<< "  |  champ:" << champAffiche
						<< "  γ=" << std::fixed << std::setprecision(2) << fieldVisuals[champAffiche].gamma
						<< "  C=" << fieldVisuals[champAffiche].contrast
						<< "  U=" << simU
						<< "  T=" << std::setprecision(3) << simTime
						<< (paused ? "  [PAUSE]" : "")
						<< (recording ? "  [REC]" : "");
					glfwSetWindowTitle(my_window, oss.str().c_str());
				}
}

// ─── Callbacks souris / clavier ───────────────────────────────────────────────

void Simulation::mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double mx, my;
		glfwGetCursorPos(w, &mx, &my);
		auto* sim = static_cast<Simulation*>(glfwGetWindowUserPointer(w));
		if (sim) sim->handleClick(mx, my);
	}
}

void Simulation::scrollCallback(GLFWwindow* w, double xoff, double yoff) {
	auto* sim = static_cast<Simulation*>(glfwGetWindowUserPointer(w));
	if (sim) sim->handleScroll(yoff);
}

void Simulation::keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;
	auto* sim = static_cast<Simulation*>(glfwGetWindowUserPointer(w));
	if (!sim) return;
	if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
	if (key == GLFW_KEY_SPACE && !sim->recording) {
		sim->paused = !sim->paused.load();
	}
}

bool Simulation::isInRect(double mx, double my, float x, float y, float w, float h) const {
	// x,y = coin haut-gauche en pixels de la fenêtre (y vers le bas)
	return mx >= x && mx <= x+w && my >= y && my <= y+h;
}

void Simulation::handleClick(double mx, double my) {
	// my_window : my_width (sim) + PANEL_W (panel)
	int fw, fh;
	glfwGetFramebufferSize(my_window, &fw, &fh);

	// glfwGetCursorPos retourne des coordonnées en pixels logiques (window),
	// mais le rendu utilise des pixels physiques (framebuffer).
	// Sur écran HiDPI/Retina le ratio peut être 2x — il faut le compenser.
	int ww, wh;
	glfwGetWindowSize(my_window, &ww, &wh);
	float scaleX = (float)fw / (float)ww;
	float scaleY = (float)fh / (float)wh;
	mx *= scaleX;
	my *= scaleY;

	float totalH = (float)fh;

	// Toutes les constantes UI sont en pixels logiques -> on les scale en pixels physiques.
	float s_margin     = margin     * scaleX;
	float s_btnH       = btnH       * scaleY;
	float s_sliderH    = sliderH    * scaleY;
	float s_sectionGap = sectionGap * scaleY;
	float s_buttonGap  = buttonGap  * scaleX;

	float px0      = (float)my_width * scaleX;
	float px1      = (float)(my_width + PANEL_W) * scaleX;
	float sliderX0 = px0 + s_margin, sliderX1 = px1 - s_margin;
	float sliderW  = sliderX1 - sliderX0;

	float curY = totalH - 20.0f * scaleY;
	curY -= s_sectionGap; // titre décoratif

	bool locked = recording;

	curY -= s_sectionGap;
	// ── 1. Sélecteur de champ ──
	float btnW = (px1 - px0 - 2.0f*s_margin - 3.0f*4.0f*scaleX) / 5.0f;
	float fieldY_top    = totalH - curY;
	float fieldY_bottom = totalH - (curY - s_btnH + 4.0f*scaleY);
	if (!locked && my >= fieldY_top && my <= fieldY_bottom) {
		for (int fi = 0; fi < (int)FIELD_NAMES.size(); fi++) {
			float bx0 = px0 + s_margin + fi*(btnW + 4.0f*scaleX);
			float bx1 = bx0 + btnW;
			if (mx >= bx0 && mx <= bx1) {
				champAffiche = FIELD_NAMES[fi];
				return;
			}
		}
	}
	curY -= s_btnH + s_sectionGap;

	// ── 2. Slider gamma ──
	float gY_top = totalH - curY, gY_bot = totalH - (curY - s_sliderH);
	if (!locked && my >= gY_top && my <= gY_bot && mx >= sliderX0 && mx <= sliderX1) {
		float norm = (float)((mx - sliderX0) / sliderW);
		norm = std::max(0.f, std::min(1.f, norm));
		fieldVisuals[champAffiche].gamma = 0.1f + norm * 1.9f;
		return;
	}
	curY -= s_sliderH + s_sectionGap;

	// ── 3. Slider contraste ──
	float cY_top = totalH - curY, cY_bot = totalH - (curY - s_sliderH);
	if (!locked && my >= cY_top && my <= cY_bot && mx >= sliderX0 && mx <= sliderX1) {
		float norm = (float)((mx - sliderX0) / sliderW);
		norm = std::max(0.f, std::min(1.f, norm));
		fieldVisuals[champAffiche].contrast = 0.5f + norm * 3.5f;
		return;
	}
	curY -= s_sliderH + s_sectionGap;

	// ── 4. START/PAUSE + STOP ──
	float halfW = (sliderW - s_buttonGap) / 2.0f;
	float btnY_top = totalH - curY, btnY_bot = totalH - (curY - s_btnH);
	if (my >= btnY_top && my <= btnY_bot) {
		// START/PAUSE
		if (mx >= sliderX0 && mx <= sliderX0 + halfW) {
			if (!locked) paused = !paused.load();
			return;
		}
		// STOP
		if (mx >= sliderX0 + halfW + s_buttonGap && mx <= sliderX1) {
			running = false;
			return;
		}
	}
	curY -= s_btnH + s_sectionGap;

	// ── 5. Slider U ──
	float uY_top = totalH - curY, uY_bot = totalH - (curY - s_sliderH);
	if (!locked && my >= uY_top && my <= uY_bot && mx >= sliderX0 && mx <= sliderX1) {
		float norm = (float)((mx - sliderX0) / sliderW);
		norm = std::max(0.f, std::min(1.f, norm));
		simU = 0.1 + norm * 4.9;
		return;
	}
	curY -= s_sliderH + s_sectionGap;

	// ── 6. Bouton enregistrement ──
	float rY_top = totalH - curY, rY_bot = totalH - (curY - s_btnH);
	if (my >= rY_top && my <= rY_bot && mx >= sliderX0 && mx <= sliderX1) {
		if (!recording) {
			recording = true;
			frameCount = 0;
			lastCaptureTime = simTime;
			openFFmpegPipes();
		} else {
			recording = false;
			closeFFmpegPipes();
		}
		return;
	}
}

void Simulation::handleScroll(double dy) {
	// Molette sur le panneau → ajuste gamma ou contraste du champ courant
	int fw, fh;
	glfwGetFramebufferSize(my_window, &fw, &fh);
	int ww, wh;
	glfwGetWindowSize(my_window, &ww, &wh);
	float scaleX = (float)fw / (float)ww;

	double mx, my;
	glfwGetCursorPos(my_window, &mx, &my);
	mx *= scaleX;
	if (mx < my_width * scaleX) return; // sur la zone simulation
	if (recording) return;

	// Shift enfoncé → contraste, sinon gamma
	bool shift = (glfwGetKey(my_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
	if (shift) {
		fieldVisuals[champAffiche].contrast =
			std::max(0.5f, std::min(4.0f, fieldVisuals[champAffiche].contrast + (float)dy * 0.1f));
	} else {
		fieldVisuals[champAffiche].gamma =
			std::max(0.1f, std::min(2.0f, fieldVisuals[champAffiche].gamma + (float)dy * -0.05f));
	}
}

// ─── Copie du champ dans le renderBuffer ──────────────────────────────────────

void Simulation::copyFieldToBuffer() {
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
		double mean = 0.0;
		for (double v : tab) mean += v;
		mean /= (double)tab.size();
		for (size_t i = 0; i < tab.size(); i++)
			renderBuffer[i] = (float)(tab[i] - mean);
	} else if (champAffiche == "vort") {
		const auto& tab = fluide.Vort().GetTab();
		renderBuffer.assign(tab.begin(), tab.end());
	} else {
		const auto& tabX = fluide.Ux().GetTab();
		const auto& tabY = fluide.Uy().GetTab();
		renderBuffer.resize(n);
		for (int i = 0; i < n; i++)
			renderBuffer[i] = (float)std::sqrt(tabX[i]*tabX[i] + tabY[i]*tabY[i]);
	}
}

// ─── Rendu principal ──────────────────────────────────────────────────────────

void Simulation::render() {
	if (newFrameReady.exchange(false)) {
		std::lock_guard<std::mutex> lock(dataMutex);

		int nx = fluide.Grid().NX();
		int ny = fluide.Grid().NY();
		int n  = nx * ny;

		float vmax_abs = 0.f;
		for (float v : renderBuffer) vmax_abs = std::max(vmax_abs, std::abs(v));
		if (vmax_abs < 1e-10f) vmax_abs = 1.0f;

		std::vector<unsigned char> tex(n);
		for (int i = 0; i < n; i++) {
			float normalized = (renderBuffer[i] + vmax_abs) / (2.f * vmax_abs);
			normalized = std::max(0.f, std::min(1.f, normalized));
			tex[i] = (unsigned char)(255.f * normalized);
		}

		glBindTexture(GL_TEXTURE_2D, dataTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny,
				GL_RED, GL_UNSIGNED_BYTE, tex.data());
	}

	int fw, fh;
	glfwGetFramebufferSize(my_window, &fw, &fh);

	glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Zone simulation (viewport à gauche)
	glViewport(0, 0, my_width, fh);
	gridShader->use();

	const FieldVisuals& fv = fieldVisuals.count(champAffiche)
		? fieldVisuals.at(champAffiche)
		: FieldVisuals{1.0f, 1.0f};

	glUniform1f(glGetUniformLocation(gridShader->ID, "uGamma"),    fv.gamma);
	glUniform1f(glGetUniformLocation(gridShader->ID, "uContrast"), fv.contrast);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dataTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, solidTexture);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Zone panneau (viewport complet pour le rendu UI en NDC globaux)
	glViewport(0, 0, fw, fh);
	renderUI();

	glfwSwapBuffers(my_window);
}

void Simulation::processInput() {
	if (glfwGetKey(my_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(my_window, true);
}

// ─── Boucle de simulation ─────────────────────────────────────────────────────

void Simulation::simulationLoop(double Tmax, double eps, int maxiter) {
	double T = 0.0;
	while (running && T < Tmax) {
		while (paused && running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
		if (!running) break;

		double dt = fluide.CFL();
		fluide.Reset_lignes();
		fluide.calc_tot_U_star(dt);
		fluide.SolveurPression(eps, dt, maxiter);
		fluide.Contrib(dt);
		fluide.condi_lim(simU);  // utilise la vitesse courante
		fluide.calc_vort();
		fluide.lignes_champ_niveau();
		T += dt;
		simTime = T;

		if (recording && (T - lastCaptureTime) >= captureInterval) {
			captureAllFieldsToPipe();
			lastCaptureTime = T;
		}

		{
			std::lock_guard<std::mutex> lock(dataMutex);
			copyFieldToBuffer();
			newFrameReady = true;
		}

		std::cout << "T = " << T << "  dt = " << dt << "\n";
	}
	running = false;
}

// ─── FFmpeg pipe ──────────────────────────────────────────────────────────────

void Simulation::openFFmpegPipes() {
	int nx = fluide.Grid().NX();
	int ny = fluide.Grid().NY();

	for (const auto& nom : FIELD_NAMES) {
		std::string cmd =
			"ffmpeg -y"
			" -f rawvideo"
			" -pix_fmt rgb24"
			" -s " + std::to_string(nx) + "x" + std::to_string(ny) +
			" -r 30"
			" -i pipe:0"
			" -c:v libx264"
			" -profile:v baseline"
			" -level 3.0"
			" -pix_fmt yuv420p"
			" -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\""
			" video_" + nom + ".mp4";

#ifdef _WIN32
		FILE* pipe = _popen(cmd.c_str(), "wb");
#else
		FILE* pipe = popen(cmd.c_str(), "w");
#endif
		if (!pipe) {
			std::cerr << "[FFMPEG] Impossible d'ouvrir le pipe pour " << nom << "\n";
		} else {
			ffmpegPipes[nom] = pipe;
			std::cout << "[FFMPEG] Pipe ouvert pour " << nom << "\n";
		}
	}
}

void Simulation::closeFFmpegPipes() {
	for (auto& [nom, pipe] : ffmpegPipes) {
		if (pipe) {
#ifdef _WIN32
			_pclose(pipe);
#else
			pclose(pipe);
#endif
			std::cout << "[FFMPEG] Pipe fermé pour " << nom << "\n";
		}
	}
	ffmpegPipes.clear();
}

// Encode une frame RGB24 pour un champ donné avec sa colormap (blue-black-red)
// + gamma/contraste propres au champ
static std::vector<unsigned char> encodeField(
		const std::vector<float>& data, int nx, int ny,
		float gamma, float contrast)
{
	int n = nx * ny;
	float vmax_abs = 0.f;
	for (float v : data) vmax_abs = std::max(vmax_abs, std::abs(v));
	if (vmax_abs < 1e-10f) vmax_abs = 1.0f;

	std::vector<unsigned char> img(n * 3);
	for (int i = 0; i < n; i++) {
		float val = (data[i] + vmax_abs) / (2.f * vmax_abs);
		val = std::max(0.f, std::min(1.f, val));

		// Contraste
		val = std::max(0.f, std::min(1.f, (val - 0.5f) * contrast + 0.5f));

		// Gamma symétrique autour de 0.5
		float c = val - 0.5f;
		float sc = (c >= 0.f) ? 1.f : -1.f;
		float ac = std::abs(c) * 2.0f;
		ac = std::pow(ac, gamma);
		val = 0.5f + sc * ac * 0.5f;
		val = std::max(0.f, std::min(1.f, val));

		// Colormap blue-black-red
		unsigned char r, g, b;
		if (val < 0.5f) {
			float t = val * 2.f;
			r = 0;
			g = 0;
			b = (unsigned char)(255.f * (1.f - t));
		} else {
			float t = (val - 0.5f) * 2.f;
			r = (unsigned char)(255.f * t);
			g = 0; b = 0;
		}
		img[i*3] = r; img[i*3+1] = g; img[i*3+2] = b;
	}

	// Flip vertical
	std::vector<unsigned char> flipped(n * 3);
	for (int y = 0; y < ny; y++)
		std::memcpy(flipped.data() + y*nx*3,
				img.data() + (ny-1-y)*nx*3, nx*3);
	return flipped;
}

void Simulation::captureAllFieldsToPipe() {
	int nx = fluide.Grid().NX();
	int ny = fluide.Grid().NY();
	int n  = nx * ny;

	std::vector<std::pair<std::string, std::vector<float>>> champs;

	{ auto& t = fluide.Ux().GetTab();
		champs.push_back({"ux", std::vector<float>(t.begin(), t.end())}); }

	{ auto& t = fluide.Uy().GetTab();
		champs.push_back({"uy", std::vector<float>(t.begin(), t.end())}); }

	{ auto& t = fluide.P().GetTab();
		std::vector<float> v(n);
		double mean = 0.0;
		for (double x : t) mean += x;
		mean /= n;
		for (int i = 0; i < n; i++) v[i] = (float)(t[i] - mean);
		champs.push_back({"p", v}); }

	{ auto& tx = fluide.Ux().GetTab();
		auto& ty = fluide.Uy().GetTab();
		std::vector<float> v(n);
		for (int i = 0; i < n; i++)
			v[i] = std::sqrt(tx[i]*tx[i] + ty[i]*ty[i]);
		champs.push_back({"u_norm", v}); }

	{ auto& t = fluide.Vort().GetTab();
		champs.push_back({"vort", std::vector<float>(t.begin(), t.end())}); }

	for (auto& [nom, data] : champs) {
		auto it = ffmpegPipes.find(nom);
		if (it == ffmpegPipes.end() || !it->second) continue;

		const FieldVisuals& fv = fieldVisuals.count(nom)
			? fieldVisuals.at(nom) : FieldVisuals{1.0f, 1.0f};

		auto frame = encodeField(data, nx, ny, fv.gamma, fv.contrast);
		fwrite(frame.data(), 1, frame.size(), it->second);
	}

	frameCount++;
}

// ─── generateVideos (no-op : les pipes sont déjà des vidéos) ──────────────────

void Simulation::generateVideos() {
	// Les vidéos ont déjà été écrites en streaming via les pipes FFmpeg.
	// On ferme proprement si ce n'est pas déjà fait.
	if (!ffmpegPipes.empty()) {
		std::cout << "Fermeture des pipes FFmpeg...\n";
		closeFFmpegPipes();
	}
	std::cout << "Vidéos générées (streaming direct, " << frameCount << " frames).\n";
}

// ─── run ──────────────────────────────────────────────────────────────────────

void Simulation::run(double Tmax, double U, double eps, int maxiter) {
	simU = U;
	if (!initGL()) return;

	running = true;
	paused  = false;
	newFrameReady = false;
	renderBuffer.resize(fluide.Grid().NX() * fluide.Grid().NY(), 0.f);

	simThread = std::thread(&Simulation::simulationLoop, this, Tmax, eps, maxiter);

	while (!glfwWindowShouldClose(my_window)) {
		processInput();
		render();
		glfwPollEvents();
	}

	running = false;
	paused  = false;
	if (simThread.joinable()) simThread.join();

	generateVideos();
}
