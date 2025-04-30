#pragma once
#include <glew.h>
#include <vector>

enum class AttributeBufferType
{
	Static,
	Dynamic,
	Resizable,
};

class AttributeBuffer
{
public:
	~AttributeBuffer();

	template<typename T>
	static AttributeBuffer* GenerateAttributeBuffer(std::vector<T>& items, std::vector<GLuint>& indices, AttributeBufferType bufferType = AttributeBufferType::Static);

	void BeginDefineAttributes();
	void EndDefineAttributes();

	void FloatAttribute(GLsizeiptr size, bool isNormalized = false);
	void IntAttribute(GLsizeiptr size, bool isUnsigned = false);
	void ShortAttribute(GLsizeiptr size, bool isUnsigned = false);

	void BindVAO();
	void UnbindVAO();

private:

	GLuint VAO = 0;
	GLuint EBO = 0;
	GLuint VBO = 0;

	AttributeBufferType VAOType = AttributeBufferType::Static;
	int ItemSize = 0;

	int VBOSize = 0;
	int VBOCount = 0;

	int EBOCount = 0;
	int EBOSize = 0;

	bool IsDefiningAttributes = false;
	int CurrentLayoutLocation = 0;
	int CurrentOffset = 0;

	AttributeBuffer(int itemsCount, int itemSize, const void* data, std::vector<GLuint>& indices, AttributeBufferType bufferType);
};

template<typename T>
AttributeBuffer* AttributeBuffer::GenerateAttributeBuffer(std::vector<T>& items, std::vector<GLuint>& indices, AttributeBufferType bufferType)
{
	AttributeBuffer* buffer = new AttributeBuffer(items.size(), sizeof(T), items.data(), indices, bufferType);
	return buffer;
}