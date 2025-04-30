#define _USE_MATH_DEFINES
#include <cmath>

#include "MeshGenerator.h"
#include "AttributeBuffer.h"

Mesh* MeshGenerator::GenerateGrid(glm::ivec2 split, glm::vec2 size, glm::vec2 offset)
{
    std::vector<PositionUV> verts = std::vector<PositionUV>();
    std::vector<GLuint> indices = std::vector<GLuint>();

    float width = size.x;
    float height = size.y;

    float offsetX = offset.x;
    float offsetY = offset.y;

    float distX = width / split.x;
    float distY = height / split.y;

    for (int y = 0; y <= split.y; ++y)
    {
        for (int x = 0; x <= split.x; ++x)
        {
            glm::vec2 position = glm::vec2(x * distX + offsetX, y * distY + offsetY);
            glm::vec2 uv = glm::vec2(x / (float)split.x, 1.0f - (y / (float)split.y));
            verts.push_back({ position, uv });
        }
    }

    for (int y = 0; y < split.y; ++y)
    {
        for (int x = 0; x < split.x; ++x)
        {
            int botLeft = y * (split.x + 1) + x;
            int botRight = botLeft + 1;
            int topLeft = botLeft + split.x + 1;
            int topRight = topLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(botRight);

            indices.push_back(topLeft);
            indices.push_back(botRight);
            indices.push_back(botLeft);
        }
    }

    return new Mesh(GenerateBuffer(verts, indices), indices.size());
}

Mesh* MeshGenerator::GenerateCircle(int segments, int circles)
{
    glm::vec2 positionOffset = { 0.5f, 0.5f };
    float radiusMultiplier = 0.5f;

    float radiusDistribution = 0.75f;
    float uvDistribution = 0.2f;

    std::vector<PositionUV> verts = std::vector<PositionUV>();
    std::vector<GLuint> indices = std::vector<GLuint>();

    verts.push_back({ positionOffset, { 0.0f, 0.0f } });

    float innerRadius = std::powf(radiusDistribution, circles - 1) * radiusMultiplier;
    float innerUvY = std::powf(uvDistribution, circles - 1);

    for (int i = 0; i < segments; ++i)
    {
        float angle = ((M_PI * 2.0f) / segments) * i;

        glm::vec2 position;
        position.x = std::sinf(angle) * innerRadius;
        position.y = std::cosf(angle) * innerRadius;

        glm::vec2 uv;
        uv.x = i % 2 == 0 ? 0.0f : 1.0f;
        uv.y = innerUvY;

        verts.push_back({ position + positionOffset, uv });

        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back((i + 1) % segments + 1);
    }

    for (int circle = 1; circle < circles; ++circle)
    {
        int offsetPreviousIndex = (circle - 1) * segments + 1;
        int offsetCurrentIndex = circle * segments + 1;

        float radius = std::powf(radiusDistribution, circles - circle - 1) * radiusMultiplier;
        float uvY = std::powf(uvDistribution, circles - circle - 1);

        for (int i = 0; i < segments; ++i)
        {
            float angle = ((M_PI * 2.0f) / segments) * i;

            glm::vec2 position;
            position.x = std::sinf(angle) * radius;
            position.y = std::cosf(angle) * radius;

            glm::vec2 uv;
            uv.x = i % 2 == 0 ? 0.0f : 1.0f;
            uv.y = uvY;

            verts.push_back({ position + positionOffset, uv });

            int innerLeft = offsetPreviousIndex + i;
            int innerRight = (i + 1) % segments + offsetPreviousIndex;
            int outterLeft = offsetCurrentIndex + i;
            int outterRight = (i + 1) % segments + offsetCurrentIndex;

            indices.push_back(innerLeft);
            indices.push_back(outterLeft);
            indices.push_back(innerRight);

            indices.push_back(innerRight);
            indices.push_back(outterLeft);
            indices.push_back(outterRight);
        }
    }

    return new Mesh(GenerateBuffer(verts, indices), indices.size());
}

AttributeBuffer* MeshGenerator::GenerateBuffer(std::vector<PositionUV>& points, std::vector<GLuint>& indices)
{
    AttributeBuffer* attributes = AttributeBuffer::GenerateAttributeBuffer(points, indices);

    attributes->BeginDefineAttributes();
    attributes->FloatAttribute(sizeof(PositionUV::Position));
    attributes->FloatAttribute(sizeof(PositionUV::UV));
    attributes->EndDefineAttributes();

    return attributes;
}