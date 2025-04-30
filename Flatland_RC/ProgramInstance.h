#pragma once
#include <string>
#include <vector>
#include <map>

#include <glew.h>
#include <glm.hpp>

class Program;

class ProgramInstance
{
public:

	ProgramInstance(Program* program);
	virtual ~ProgramInstance();

	void Bind();
	void Unbind();

	void SetBool(GLuint id, bool value);
	void SetBool(std::string name, bool value);

	void SetInt(GLuint id, int value);
	void SetInt(std::string name, int value);

	void SetFloat(GLuint id, float value);
	void SetFloat(std::string name, float value);

	void SetVec2(GLuint id, glm::vec2 value);
	void SetVec2(std::string name, glm::vec2 value);

	void SetVec3(GLuint id, glm::vec3 value);
	void SetVec3(std::string name, glm::vec3 value);

	void SetVec4(GLuint id, glm::vec4 value);
	void SetVec4(std::string name, glm::vec4 value);

	void SetMat3(GLuint id, const glm::mat3x3& value);
	void SetMat3(std::string name, const glm::mat3x3& value);

	void SetMat4(GLuint id, const glm::mat4x4& value);
	void SetMat4(std::string name, const glm::mat4x4& value);

	void SetTexture(GLuint id, GLuint textureID);
	void SetTexture(std::string name, GLuint textureID);

	Program* GetProgram();
	GLuint GetUniformID(const std::string& name);

protected:

	Program* ConnectedProgram = nullptr;

private:

	std::map<GLuint, bool> BoolUniformValues;
	std::map<GLuint, int> IntUniformValues;
	std::map<GLuint, float> FloatUniformValues;
	std::map<GLuint, glm::vec2> Vec2UniformValues;
	std::map<GLuint, glm::vec3> Vec3UniformValues;
	std::map<GLuint, glm::vec4> Vec4UniformValues;
	std::map<GLuint, glm::mat3x3> Mat3UniformValues;
	std::map<GLuint, glm::mat4x4> Mat4UniformValues;
	std::map<GLuint, GLuint> TextureUniformValues;

	template<typename T>
	void SetUniform(std::map<GLuint, T>& values, GLuint id, T value);

};
