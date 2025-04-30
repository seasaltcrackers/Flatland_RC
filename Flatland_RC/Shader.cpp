#include <vector>
#include <fstream>

#include "Shader.h"

Shader* Shader::GenerateFromSource(std::string name, GLenum shaderType, const std::string& source)
{
	// Read the file
	const char* sourceChar = source.c_str();
	const GLint sourceSize = source.size();

	// Create and set the shader source
	GLuint shaderID = glCreateShader(shaderType);
	glShaderSource(shaderID, 1, &sourceChar, &sourceSize);
	glCompileShader(shaderID);

	// Check for errors
	int compile_result = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compile_result);

	if (compile_result == GL_FALSE)
		printf(GetShaderErrorLog(name, shaderID, shaderType).c_str());

	return new Shader(name, shaderID);
}

Shader* Shader::GenerateFromFilename(GLenum shaderType, const std::string& filepath)
{
	// Read contents of file
	std::ifstream file(filepath, std::ios::in);
	std::string fileContents;

	// Ensure the file is open and readable
	if (!file.good())
		printf("Cannot find file at path: '{0}'", filepath);

	// Determine the size of of the file in characters and resize the string variable to accomodate
	file.seekg(0, std::ios::end);
	fileContents.resize((unsigned int)file.tellg());

	// Set the position of the next character to be read back to the beginning
	file.seekg(0, std::ios::beg);

	// Extract the contents of the file and store in the string variable
	file.read(&fileContents[0], fileContents.size());
	file.close();

	return GenerateFromSource(filepath, shaderType, fileContents);
}

std::string Shader::GetShaderErrorLog(const std::string& name, GLuint shaderID, GLenum shaderType)
{
	int infoLogLength;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

	std::string shaderTypeStr = "Unknown";

	if (shaderType == GL_VERTEX_SHADER)
		shaderTypeStr = "Vertex Shader";
	else if (shaderType == GL_FRAGMENT_SHADER)
		shaderTypeStr = "Fragment Shader";
	else if (shaderType == GL_COMPUTE_SHADER)
		shaderTypeStr = "Compute Shader";

	if (infoLogLength != 0)
	{
		// Retrieve the log info and populate log variable
		std::vector<char> log(infoLogLength);
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, &log[0]);

		std::string errorLog(log.begin(), log.end());
		return "Error compiling " + name + " " + shaderTypeStr + ":\n" + errorLog;
	}
	else
	{
		return "Unknown Error compiling " + name + " " + shaderTypeStr;
	}
}

Shader::Shader(std::string name, GLuint shaderID)
{
	ShaderID = shaderID;
	Name = name;
}

Shader::~Shader()
{
	glDeleteShader(ShaderID);
}
