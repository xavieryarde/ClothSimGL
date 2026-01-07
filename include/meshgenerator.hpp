#include <vector>
#include <glm/glm.hpp>

struct PoleVertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

namespace MeshGenerator
{
    std::vector<PoleVertex> generateCylinder(float radius, float height, int slices);
    std::vector<PoleVertex> generateCube(float size);
    std::vector<PoleVertex> generateSphere(float radius, int rings, int sectors);
}