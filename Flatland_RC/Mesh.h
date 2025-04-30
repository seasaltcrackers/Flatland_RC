#pragma once
#include <glm.hpp>
#include <vector>
#include <glew.h>

class AttributeBuffer;

class Mesh
{
public:

	Mesh(AttributeBuffer* vertexAttributes, int indiceCount);
	~Mesh();

	void RenderMesh();
	void RenderMeshInstanced(int amount);

private:

	int IndiceCount = 0;
	AttributeBuffer* VertexAttributes = nullptr;

};

