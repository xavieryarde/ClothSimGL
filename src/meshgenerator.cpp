#include "meshgenerator.hpp"

namespace MeshGenerator
{
    std::vector<PoleVertex> generateCylinder(float radius, float height, int slices)
    {
        constexpr float PI = 3.14159265359f;
        std::vector<PoleVertex> vertices;

        for (int i = 0; i < slices; i++)
        {
            float theta = 2.0f * PI * i / slices;
            float nextTheta = 2.0f * PI * (i + 1) / slices;

            float x0 = radius * cos(theta), z0 = radius * sin(theta);
            float x1 = radius * cos(nextTheta), z1 = radius * sin(nextTheta);

            float y0 = 0.0f, y1 = height;

            glm::vec3 n0 = glm::normalize(glm::vec3(cos(theta), 0.0f, sin(theta)));
            glm::vec3 n1 = glm::normalize(glm::vec3(cos(nextTheta), 0.0f, sin(nextTheta)));

            // First triangle
            vertices.push_back({ glm::vec3(x0, y0, z0), n0 });
            vertices.push_back({ glm::vec3(x1, y0, z1), n1 });
            vertices.push_back({ glm::vec3(x1, y1, z1), n1 });

            // Second triangle
            vertices.push_back({ glm::vec3(x0, y0, z0), n0 });
            vertices.push_back({ glm::vec3(x1, y1, z1), n1 });
            vertices.push_back({ glm::vec3(x0, y1, z0), n0 });
        }

        return vertices;
    }

    std::vector<PoleVertex> generateCube(float size)
    {
        std::vector<PoleVertex> vertices;
        float half = size * 0.5f;

        // Define cube faces with normals
        std::vector<glm::vec3> positions = {
            // Front face
            {-half, -half,  half}, { half, -half,  half}, { half,  half,  half},
            {-half, -half,  half}, { half,  half,  half}, {-half,  half,  half},
            // Back face
            { half, -half, -half}, {-half, -half, -half}, {-half,  half, -half},
            { half, -half, -half}, {-half,  half, -half}, { half,  half, -half},
            // Left face
            {-half, -half, -half}, {-half, -half,  half}, {-half,  half,  half},
            {-half, -half, -half}, {-half,  half,  half}, {-half,  half, -half},
            // Right face
            { half, -half,  half}, { half, -half, -half}, { half,  half, -half},
            { half, -half,  half}, { half,  half, -half}, { half,  half,  half},
            // Bottom face
            {-half, -half, -half}, { half, -half, -half}, { half, -half,  half},
            {-half, -half, -half}, { half, -half,  half}, {-half, -half,  half},
            // Top face
            {-half,  half,  half}, { half,  half,  half}, { half,  half, -half},
            {-half,  half,  half}, { half,  half, -half}, {-half,  half, -half}
        };

        std::vector<glm::vec3> normals = {
            // Front face
            {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
            {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
            // Back face
            {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
            {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
            // Left face
            {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
            {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
            // Right face
            {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
            {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
            // Bottom face
            {0, -1, 0}, {0, -1, 0}, {0, -1, 0},
            {0, -1, 0}, {0, -1, 0}, {0, -1, 0},
            // Top face
            {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
            {0, 1, 0}, {0, 1, 0}, {0, 1, 0}
        };

        for (size_t i = 0; i < positions.size(); ++i)
        {
            vertices.push_back({ positions[i], normals[i] });
        }

        return vertices;
    }

    std::vector<PoleVertex> generateSphere(float radius, int rings, int sectors)
    {
        constexpr float PI = 3.14159265359f;
        std::vector<PoleVertex> vertices;

        for (int i = 0; i <= rings; ++i)
        {
            float phi = PI * i / rings;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);

            for (int j = 0; j <= sectors; ++j)
            {
                float theta = 2.0f * PI * j / sectors;
                float cosTheta = cos(theta);
                float sinTheta = sin(theta);

                glm::vec3 pos = {
                    radius * sinPhi * cosTheta,
                    radius * cosPhi,
                    radius * sinPhi * sinTheta
                };

                glm::vec3 normal = glm::normalize(pos);

                if (i < rings && j < sectors)
                {
                    int first = i * (sectors + 1) + j;
                    int second = first + sectors + 1;

                    // First triangle
                    vertices.push_back({ pos, normal });

                    // Calculate next positions
                    float nextPhi = PI * (i + 1) / rings;
                    float nextTheta = 2.0f * PI * (j + 1) / sectors;

                    glm::vec3 pos1 = {
                        radius * sin(nextPhi) * cos(theta),
                        radius * cos(nextPhi),
                        radius * sin(nextPhi) * sin(theta)
                    };

                    glm::vec3 pos2 = {
                        radius * sin(nextPhi) * cos(nextTheta),
                        radius * cos(nextPhi),
                        radius * sin(nextPhi) * sin(nextTheta)
                    };

                    vertices.push_back({ pos1, glm::normalize(pos1) });
                    vertices.push_back({ pos2, glm::normalize(pos2) });

                    // Second triangle
                    vertices.push_back({ pos, normal });
                    vertices.push_back({ pos2, glm::normalize(pos2) });

                    glm::vec3 pos3 = {
                        radius * sinPhi * cos(nextTheta),
                        radius * cosPhi,
                        radius * sinPhi * sin(nextTheta)
                    };

                    vertices.push_back({ pos3, glm::normalize(pos3) });
                }
            }
        }

        return vertices;
    }
}