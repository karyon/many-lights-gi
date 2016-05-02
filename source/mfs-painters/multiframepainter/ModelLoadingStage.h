#pragma once

#include <vector>
#include <memory>

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
    class Drawable;
    class ResourceManager;
    class Scene;
}

class aiMesh;
class aiScene;
class aiMaterial;

class ModelLoadingStage
{
public:
    ModelLoadingStage(Preset preset);
    ~ModelLoadingStage();

    gloperate::ResourceManager* resourceManager;
    Preset preset;

    void process();

    const PresetInformation& getCurrentPreset() const;
    const IdDrawablesMap& getDrawablesMap() const;
    const IdMaterialMap& getMaterialMap() const;


protected:
    using StringTextureMap = std::map<std::string, globjects::ref_ptr<globjects::Texture>>;

    float m_maxAnisotropy;
    StringTextureMap m_textures;


    globjects::ref_ptr<globjects::Texture> loadTexture(const std::string& filename) const;
    Material loadMaterial(aiMaterial* mat, const std::string& directory);
    gloperate::Scene * convertScene(const aiScene * scene) const;
    gloperate::PolygonalGeometry * convertGeometry(const aiMesh * mesh) const;

    static PresetInformation getPresetInformation(Preset preset);
    static std::string getFilename(Preset preset);


    std::unique_ptr<PresetInformation> m_presetInformation;
    std::unique_ptr<IdDrawablesMap> m_drawablesMap;
    std::unique_ptr<IdMaterialMap> m_materialMap;
};
