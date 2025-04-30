#pragma once
#include <map>
#include <string>
#include <vector>
#include <glew.h>

#include "Mesh.h"

struct PositionUV
{
	glm::vec2 Position = { 0.0f, 0.0f };
	glm::vec2 UV = { 0.0f, 0.0f };
};

class MeshGenerator
{
public:

	static Mesh* GenerateGrid(glm::ivec2 split, glm::vec2 size = { 1.0f, 1.0f }, glm::vec2 offset = { 0.0f, 0.0f });
	static Mesh* GenerateCircle(int segments, int circles);

private:

	MeshGenerator() {}
	~MeshGenerator() {}

	static AttributeBuffer* GenerateBuffer(std::vector<PositionUV>& points, std::vector<GLuint>& indices);

};

