#include "ProgramInstance.h"
#include "Program.h"

template<typename T>
inline void ProgramInstance::SetUniform(std::map<GLuint, T>& values, GLuint id, T value)
{
	if (values.find(id) == values.end())
	{
		values.insert({ id, value });
		return;
	}

	values[id] = value;
}

ProgramInstance::ProgramInstance(Program* program)
{
	ConnectedProgram = program;
}

ProgramInstance::~ProgramInstance()
{
}

void ProgramInstance::Bind()
{
	ConnectedProgram->BindProgram();

	for (auto uniformValue : TextureUniformValues)
		ConnectedProgram->SetTexture(uniformValue.first, uniformValue.second);

	for (auto uniformValue : BoolUniformValues)
		ConnectedProgram->SetBool(uniformValue.first, uniformValue.second);

	for (auto uniformValue : IntUniformValues)
		ConnectedProgram->SetInt(uniformValue.first, uniformValue.second);

	for (auto uniformValue : FloatUniformValues)
		ConnectedProgram->SetFloat(uniformValue.first, uniformValue.second);

	for (const auto& uniformValue : Vec2UniformValues)
		ConnectedProgram->SetVector(uniformValue.first, uniformValue.second);

	for (const auto& uniformValue : Vec3UniformValues)
		ConnectedProgram->SetVector(uniformValue.first, uniformValue.second);

	for (const auto& uniformValue : Vec4UniformValues)
		ConnectedProgram->SetVector(uniformValue.first, uniformValue.second);

	for (const auto& uniformValue : Mat3UniformValues)
		ConnectedProgram->SetMatrix(uniformValue.first, uniformValue.second);

	for (const auto& uniformValue : Mat4UniformValues)
		ConnectedProgram->SetMatrix(uniformValue.first, uniformValue.second);
}

void ProgramInstance::Unbind()
{
	ConnectedProgram->UnbindProgram();
}

void ProgramInstance::SetBool(GLuint id, bool value)
{
	SetUniform(BoolUniformValues, id, value);
}

void ProgramInstance::SetBool(std::string name, bool value)
{
	SetUniform(BoolUniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetInt(GLuint id, int value)
{
	SetUniform(IntUniformValues, id, value);
}

void ProgramInstance::SetInt(std::string name, int value)
{
	SetUniform(IntUniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetFloat(GLuint id, float value)
{
	SetUniform(FloatUniformValues, id, value);
}

void ProgramInstance::SetFloat(std::string name, float value)
{
	SetUniform(FloatUniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetVec2(GLuint id, glm::vec2 value)
{
	SetUniform(Vec2UniformValues, id, value);
}

void ProgramInstance::SetVec2(std::string name, glm::vec2 value)
{
	SetUniform(Vec2UniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetVec3(GLuint id, glm::vec3 value)
{
	SetUniform(Vec3UniformValues, id, value);
}

void ProgramInstance::SetVec3(std::string name, glm::vec3 value)
{
	SetUniform(Vec3UniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetVec4(GLuint id, glm::vec4 value)
{
	SetUniform(Vec4UniformValues, id, value);
}

void ProgramInstance::SetVec4(std::string name, glm::vec4 value)
{
	SetUniform(Vec4UniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetMat3(GLuint id, const glm::mat3x3& value)
{
	SetUniform(Mat3UniformValues, id, value);
}

void ProgramInstance::SetMat3(std::string name, const glm::mat3x3& value)
{
	SetUniform(Mat3UniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetMat4(GLuint id, const glm::mat4x4& value)
{
	SetUniform(Mat4UniformValues, id, value);
}

void ProgramInstance::SetMat4(std::string name, const glm::mat4x4& value)
{
	SetUniform(Mat4UniformValues, GetUniformID(name), value);
}

void ProgramInstance::SetTexture(GLuint id, GLuint textureID)
{
	SetUniform(TextureUniformValues, id, textureID);
}

void ProgramInstance::SetTexture(std::string name, GLuint textureID)
{
	SetUniform(TextureUniformValues, GetUniformID(name), textureID);
}

Program* ProgramInstance::GetProgram()
{
	return ConnectedProgram;
}

GLuint ProgramInstance::GetUniformID(const std::string& name)
{
	return ConnectedProgram->GetUniformID(name);
}
