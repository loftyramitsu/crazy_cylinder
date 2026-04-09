#pragma once
#include <string>

class Shader
{
	public:
		unsigned int ID;
		
		Shader() : ID(0) {};
		Shader(const std::string& vertexPath, const std::string& fragmentPath);
		void use() const;
};
