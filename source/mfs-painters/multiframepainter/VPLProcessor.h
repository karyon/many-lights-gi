#pragma once

#include <glm/mat4x4.hpp>

#include <globjects/base/ref_ptr.h>

namespace globjects
{
    class Buffer;
    class Program;
}

class RasterizationStage;


class VPLProcessor
{
public:
    VPLProcessor(const RasterizationStage& rsmRenderer);
    ~VPLProcessor();

    void process();

    globjects::ref_ptr<globjects::Buffer> vplBuffer;
    glm::mat4 biasedShadowTransform;

private:
    globjects::ref_ptr<globjects::Program> m_program;

    const RasterizationStage& m_rsmRenderer;
};
