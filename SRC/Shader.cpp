#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "[SHADER ERROR] Impossible d'ouvrir : " << path << "\n";
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	std::string vCode = readFile(vertexPath);
	std::string fCode = readFile(fragmentPath);

	const char* vShaderCode = vCode.c_str();
	const char* fShaderCode = fCode.c_str();

	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, nullptr);
	glCompileShader(vertex);

	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, nullptr);
	glCompileShader(fragment);

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() const
{
	glUseProgram(ID);
}






