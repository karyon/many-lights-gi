#pragma once

#include <vector>
#include <memory>
#include <map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Material.h"

namespace globjects
{
    class Texture;
}

namespace gloperate
{
    class PolygonalDrawable;
}

using PolygonalDrawablePointer = std::unique_ptr<gloperate::PolygonalDrawable>;
using PolygonalDrawables = std::vector<PolygonalDrawablePointer>;
using IdMaterialMap = std::map<unsigned int, Material>;
using IdDrawablesMap = std::map<unsigned int, PolygonalDrawables>;

struct PresetInformation
{
    glm::vec3 camEye;
    glm::vec3 camCenter;
    glm::vec2 nearFar;
    glm::vec3 lightPosition;
    glm::vec3 lightCenter;
    float lightMaxShift;
    glm::vec3 groundColor;
    float groundHeight;
    float alpha;
    BumpType bumpType;
    bool useReflections;
    float zThickness;
    float focalDist;
    float focalPoint;
    float vertexScale;
};
