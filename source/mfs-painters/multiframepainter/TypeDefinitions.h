#pragma once

#include <vector>
#include <memory>
#include <map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <gloperate/primitives/Icosahedron.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/PolygonalGeometry.h>

#include "Material.h"


class PolygonalDrawable;

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


using PolygonalDrawablePointer = std::unique_ptr<PolygonalDrawable>;
using PolygonalDrawables = std::vector<PolygonalDrawablePointer>;
using IdMaterialMap = std::map<unsigned int, Material>;
using IdDrawablesMap = std::map<unsigned int, PolygonalDrawables>;



class PolygonalDrawable : public gloperate::PolygonalDrawable
{
public:
    PolygonalDrawable() : gloperate::PolygonalDrawable(gloperate::PolygonalGeometry()){};
    PolygonalDrawable(const gloperate::PolygonalGeometry & geometry) : gloperate::PolygonalDrawable(geometry) {};
    virtual ~PolygonalDrawable() = default;

    glm::mat4 modelMatrix;
};



class Icosahedron : public PolygonalDrawable
{
public:
    Icosahedron() : PolygonalDrawable() {
        ico = globjects::make_ref<gloperate::Icosahedron>(2);
    }

    void draw() override {
        ico->draw();
    }
    void draw(gl::GLenum primitive) override {
        ico->draw(primitive);
    }

private:
    globjects::ref_ptr<gloperate::Icosahedron> ico;
};


