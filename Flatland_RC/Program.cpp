#include <stdexcept>
#include <gtc\type_ptr.hpp>

#include "Program.h"
#include "Shader.h"

Program::Program(GLuint programID)
{
	ProgramID = programID;
	IsBound = false;
}

Program::~Program()
{
	glDeleteProgram(ProgramID);
}

void Program::BindProgram()
{
	glUseProgram(ProgramID);
	IsBound = true;
}

void Program::UnbindProgram()
{
	for (int i = 0; i < TextureCount; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glUseProgram(0);
	IsBound = false;
	TextureCount = 0;
}

GLuint Program::GetUniformID(const std::string& name)
{
	return glGetUniformLocation(ProgramID, name.c_str());
}

GLuint Program::GetProgramID()
{
	return ProgramID;
}

void Program::SetBool(GLuint id, bool value)
{
	ValidateSetUniform(id);
	glUniform1i(id, value ? 1 : 0);
}

void Program::SetBool(const std::string& name, bool value)
{
	ValidateSetUniform(name);
	glUniform1i(GetUniformID(name), value ? 1 : 0);
}

void Program::SetFloat(GLuint id, float value)
{
	ValidateSetUniform(id);
	glUniform1f(id, value);
}

void Program::SetInt(GLuint id, int value)
{
	ValidateSetUniform(id);
	glUniform1i(id, value);
}

void Program::SetFloat(const std::string& name, float value)
{
	ValidateSetUniform(name);
	SetFloat(GetUniformID(name), value);
}

void Program::SetInt(const std::string& name, int value)
{
	ValidateSetUniform(name);
	SetInt(GetUniformID(name), value);
}

void Program::SetVector(GLuint id, glm::vec2 value)
{
	ValidateSetUniform(id);
	glUniform2f(id, value.x, value.y);
}

void Program::SetVector(GLuint id, glm::vec3 value)
{
	ValidateSetUniform(id);
	glUniform3f(id, value.x, value.y, value.z);
}

void Program::SetVector(GLuint id, glm::vec4 value)
{
	ValidateSetUniform(id);
	glUniform4f(id, value.x, value.y, value.z, value.w);
}

void Program::SetVector(const std::string& name, glm::vec2 value)
{
	ValidateSetUniform(name);
	SetVector(GetUniformID(name), value);
}

void Program::SetVector(const std::string& name, glm::vec3 value)
{
	ValidateSetUniform(name);
	SetVector(GetUniformID(name), value);
}

void Program::SetVector(const std::string& name, glm::vec4 value)
{
	ValidateSetUniform(name);
	SetVector(GetUniformID(name), value);
}

void Program::SetMatrix(GLuint id, const glm::mat3x3& value)
{
	ValidateSetUniform(id);
	glUniformMatrix3fv(id, 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetMatrix(GLuint id, const glm::mat4x4& value)
{
	ValidateSetUniform(id);
	glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetMatrix(const std::string& name, const glm::mat3x3& value)
{
	ValidateSetUniform(name);
	SetMatrix(GetUniformID(name), value);
}

void Program::SetMatrix(const std::string& name, const glm::mat4x4& value)
{
	ValidateSetUniform(name);
	SetMatrix(GetUniformID(name), value);
}

void Program::SetTexture(GLuint id, GLuint textureID)
{
	ValidateSetUniform(id);

	glActiveTexture(GL_TEXTURE0 + TextureCount);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glUniform1i(id, TextureCount);
	TextureCount++;
}

void Program::SetTexture(const std::string& name, GLuint textureID)
{
	ValidateSetUniform(name);

	glActiveTexture(GL_TEXTURE0 + TextureCount);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glUniform1i(GetUniformID(name), TextureCount);
	TextureCount++;
}

void Program::ValidateSetUniform(GLuint id)
{
	if (!IsBound)
		printf("Trying to set Program variable while unbound");
}

void Program::ValidateSetUniform(const std::string& name)
{
	if (!IsBound)
		printf("Trying to set Program variable while unbound");
}

Program* Program::GenerateFromFileVsFs(const std::string& filepathVs, const std::string& filepathFs)
{
	Shader* vsShader = Shader::GenerateFromFilename(GL_VERTEX_SHADER, filepathVs);
	Shader* fsShader = Shader::GenerateFromFilename(GL_FRAGMENT_SHADER, filepathFs);
	return GenerateProgram({ vsShader, fsShader });
}

Program* Program::GenerateFromFileCs(const std::string& filepathCs)
{
	Shader* csShader = Shader::GenerateFromFilename(GL_COMPUTE_SHADER, filepathCs);
	return GenerateProgram({ csShader });
}

Program* Program::GenerateProgram(std::vector<Shader*> shaders)
{
	// Generate and attach the shaders to the program
	GLuint programID = glCreateProgram();

	for (Shader* shader : shaders)
		glAttachShader(programID, shader->ShaderID);

	glLinkProgram(programID);

	// Check for link errors
	int link_result = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &link_result);

	if (link_result == GL_FALSE)
		printf(GetProgramErrorLog(shaders, programID).c_str());

	return new Program(programID);
}

std::string Program::GetProgramErrorLog(const std::vector<Shader*>& shaders, GLuint programID)
{
	std::string name = shaders.size() > 0 ? shaders[0]->Name : "";

	for (int i = 1; i < shaders.size(); ++i)
		name += ", " + shaders[i]->Name;

	int infoLogLength;
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength != 0)
	{
		// Retrieve the log info and populate log variable
		std::vector<char> log(infoLogLength);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &log[0]);

		std::string errorLog(log.begin(), log.end());
		return "Program Error Linking Shaders: " + name + "\n" + errorLog;
	}
	else
	{
		return "Program Unknown Error Linking Shaders: " + name;
	}
}
