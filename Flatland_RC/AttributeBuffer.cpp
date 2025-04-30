#include "AttributeBuffer.h"
#include <cassert>
#include <string>

AttributeBuffer::~AttributeBuffer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
}

void AttributeBuffer::BeginDefineAttributes()
{
	BindVAO();
	IsDefiningAttributes = true;
	CurrentOffset = 0;
}

void AttributeBuffer::EndDefineAttributes()
{
	if (CurrentOffset != ItemSize)
		printf("Expected {0} bytes, but only defined {1} bytes of attributes.", ItemSize, CurrentOffset);

	IsDefiningAttributes = false;
	UnbindVAO();
}

void AttributeBuffer::FloatAttribute(GLsizeiptr size, bool isNormalized)
{
	if (!IsDefiningAttributes)
		printf("Tried defining attribute without calling BeginDefineAttributes() first");

	glEnableVertexAttribArray(CurrentLayoutLocation);
	glVertexAttribPointer(CurrentLayoutLocation, size / sizeof(float), GL_FLOAT, isNormalized ? GL_TRUE : GL_FALSE, ItemSize, (GLvoid*)CurrentOffset);

	CurrentOffset += size;
	CurrentLayoutLocation++;
}

void AttributeBuffer::IntAttribute(GLsizeiptr size, bool isUnsigned)
{
	if (!IsDefiningAttributes)
		printf("Tried defining attribute without calling BeginDefineAttributes() first");

	glEnableVertexAttribArray(CurrentLayoutLocation);
	glVertexAttribIPointer(CurrentLayoutLocation, size / sizeof(int), isUnsigned ? GL_UNSIGNED_INT : GL_INT, ItemSize, (GLvoid*)CurrentOffset);

	CurrentOffset += size;
	CurrentLayoutLocation++;
}

void AttributeBuffer::ShortAttribute(GLsizeiptr size, bool isUnsigned)
{
	if (!IsDefiningAttributes)
		printf("Tried defining attribute without calling BeginDefineAttributes() first");

	glEnableVertexAttribArray(CurrentLayoutLocation);
	glVertexAttribIPointer(CurrentLayoutLocation, size / sizeof(short), isUnsigned ? GL_UNSIGNED_SHORT : GL_SHORT, ItemSize, (GLvoid*)CurrentOffset);

	CurrentOffset += size;
	CurrentLayoutLocation++;
}

void AttributeBuffer::BindVAO()
{
	glBindVertexArray(VAO);
}

void AttributeBuffer::UnbindVAO()
{
	glBindVertexArray(0);
}

AttributeBuffer::AttributeBuffer(int itemsCount, int itemSize, const void* data, std::vector<GLuint>& indices, AttributeBufferType bufferType)
{
	VAOType = bufferType;
	ItemSize = itemSize;

	VBOCount = itemsCount;
	VBOSize = itemsCount;

	EBOSize = indices.size();
	EBOCount = indices.size();


	if (bufferType == AttributeBufferType::Resizable)
	{
		VBOSize = 1;
		EBOSize = 1;

		while (VBOCount > VBOSize)
			VBOSize *= 2;

		while (EBOCount > EBOSize)
			EBOSize *= 2;
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBOSize * sizeof(GLuint), indices.data(), VAOType == AttributeBufferType::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, VBOSize * itemSize, data, VAOType == AttributeBufferType::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}