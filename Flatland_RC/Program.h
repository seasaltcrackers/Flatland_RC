#pragma once
#include <glm.hpp>
#include <glew.h>
#include <string>
#include <vector>

class Shader;

class Program
{
public:
	static Program* GenerateFromFileVsFs(const std::string& filepathVs, const std::string& filepathFs);
	static Program* GenerateFromFileCs(const std::string& filepathCs);
	static Program* GenerateProgram(std::vector<Shader*> shaders);

	Program(GLuint programID);
	~Program();

	void BindProgram();
	void UnbindProgram();

	GLuint GetUniformID(const std::string& name);
	GLuint GetProgramID();

	void SetBool(GLuint id, bool value);
	void SetBool(const std::string& name, bool value);

	void SetInt(GLuint id, int value);
	void SetInt(const std::string& name, int value);

	void SetFloat(GLuint id, float value);
	void SetFloat(const std::string& name, float value);

	void SetVector(GLuint id, glm::vec2 value);
	void SetVector(GLuint id, glm::vec3 value);
	void SetVector(GLuint id, glm::vec4 value);

	void SetVector(const std::string& name, glm::vec2 value);
	void SetVector(const std::string& name, glm::vec3 value);
	void SetVector(const std::string& name, glm::vec4 value);

	void SetMatrix(GLuint id, const glm::mat3x3& value);
	void SetMatrix(GLuint id, const glm::mat4x4& value);

	void SetMatrix(const std::string& name, const glm::mat3x3& value);
	void SetMatrix(const std::string& name, const glm::mat4x4& value);

	void SetTexture(GLuint id, GLuint textureID);
	void SetTexture(const std::string& name, GLuint textureID);

private:

	static std::string GetProgramErrorLog(const std::vector<Shader*>& shaders, GLuint programID);

	GLuint ProgramID = 0;
	bool IsBound = false;

	int TextureCount = 0;

	void ValidateSetUniform(GLuint id);
	void ValidateSetUniform(const std::string& name);
};

