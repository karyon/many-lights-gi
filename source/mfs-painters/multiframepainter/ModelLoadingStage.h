#pragma once

#include <vector>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/pipeline/Data.h>

#include "TypeDefinitions.h"
#include "Preset.h"
#include "Material.h"

namespace globjects
{
    class Texture;
}

namespace gloperate
{
    class PolygonalGeometry;
    class PolygonalDrawable;
    class ResourceManager;
    class Scene;
}

class aiMesh;
class aiScene;
class aiMaterial;

class ModelLoadingStage : public gloperate::AbstractStage
{
public:
    ModelLoadingStage();

    gloperate::InputSlot<gloperate::ResourceManager*> resourceManager;
    gloperate::InputSlot<Preset> preset;

    gloperate::Data<PresetInformation> presetInformation;
    gloperate::Data<IdDrawablesMap> drawablesMap;
    gloperate::Data<IdMaterialMap> materialMap;

protected:
    using StringTextureMap = std::map<std::string, globjects::ref_ptr<globjects::Texture>>;

    float m_maxAnisotropy;
    StringTextureMap m_textures;

    virtual void process() override;

    globjects::ref_ptr<globjects::Texture> loadTexture(const std::string& filename) const;
    Material loadMaterial(aiMaterial* mat, const std::string& directory);
    gloperate::Scene * convertScene(const aiScene * scene) const;
    gloperate::PolygonalGeometry * convertGeometry(const aiMesh * mesh) const;

    static PresetInformation getPresetInformation(Preset preset);
    static std::string getFilename(Preset preset);
};
