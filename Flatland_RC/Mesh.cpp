#include "Mesh.h"
#include "AttributeBuffer.h"

Mesh::~Mesh()
{
	delete VertexAttributes;
	VertexAttributes = nullptr;
}

Mesh::Mesh(AttributeBuffer* vertexAttributes, int indiceCount)
{
	VertexAttributes = vertexAttributes;
	IndiceCount = indiceCount;
}

void Mesh::RenderMesh()
{
	VertexAttributes->BindVAO();
	glDrawElements(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_INT, 0);
	VertexAttributes->UnbindVAO();
}

void Mesh::RenderMeshInstanced(int amount)
{
	VertexAttributes->BindVAO();
	glDrawElementsInstanced(GL_TRIANGLES, IndiceCount, GL_UNSIGNED_INT, 0, amount);
	VertexAttributes->UnbindVAO();
}