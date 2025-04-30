#pragma once
#include <string>
#include <glew.h>

class Shader
{
public:

	static Shader* GenerateFromSource(std::string programName, GLenum shaderType, const std::string& source);
	static Shader* GenerateFromFilename(GLenum shaderType, const std::string& filepath);
	static std::string GetShaderErrorLog(const std::string& programName, GLuint shaderID, GLenum shaderType);

	GLuint ShaderID = 0;
	std::string Name = "";

	Shader(std::string name, GLuint shaderID);
	~Shader();

};

