#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

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
    ModelLoadingStage();
    ~ModelLoadingStage();

    gloperate::ResourceManager* resourceManager;

    void loadScene(Preset preset);

    const Preset& getCurrentPreset() const;
    const PresetInformation& getCurrentPresetInformation() const;
    const IdDrawablesMap& getDrawablesMap() const;
    const IdMaterialMap& getMaterialMap() const;


protected:
    using StringTextureMap = std::map<std::string, globjects::ref_ptr<globjects::Texture>>;

    float m_maxAnisotropy;
    StringTextureMap m_textures;


    globjects::ref_ptr<globjects::Texture> loadTexture(const std::string& filename) const;
    Material loadMaterial(aiMaterial* mat, const std::string& directory);
    std::unique_ptr<gloperate::PolygonalGeometry> convertGeometry(const aiMesh * mesh, float vertexScale) const;

    static PresetInformation getPresetInformation(Preset preset);
    static std::string getFilename(Preset preset);


    Preset m_currentPreset;
    std::unique_ptr<PresetInformation> m_currentPresetInformation;
    std::unique_ptr<IdDrawablesMap> m_drawablesMap;
    std::unique_ptr<IdMaterialMap> m_materialMap;
};
