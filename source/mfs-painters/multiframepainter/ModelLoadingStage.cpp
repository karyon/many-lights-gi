#include "ModelLoadingStage.h"

#include <iostream>
#include <algorithm>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>

#include <globjects/Texture.h>

#include <gloperate/primitives/PolygonalGeometry.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/base/make_unique.hpp>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

using namespace gl;
using gloperate::make_unique;

namespace
{
    std::string getDirectory(const std::string& path)
    {
        size_t found = path.find_last_of("/\\");
        return(path.substr(0, found));
    }

    auto textureTypes = {
        aiTextureType_DIFFUSE,
        aiTextureType_EMISSIVE,
        aiTextureType_SPECULAR,
        aiTextureType_HEIGHT,
        aiTextureType_OPACITY
    };

    TextureType translateAssimpTextureType(aiTextureType aiTexType)
    {
        static const std::map<aiTextureType, TextureType> conversion {
            { aiTextureType_DIFFUSE, TextureType::Diffuse },
            { aiTextureType_SPECULAR, TextureType::Specular },
            { aiTextureType_EMISSIVE, TextureType::Emissive },
            { aiTextureType_HEIGHT, TextureType::Bump },
            { aiTextureType_OPACITY, TextureType::Opacity }
        };

        return conversion.at(aiTexType);
    }
}

ModelLoadingStage::ModelLoadingStage(Preset preset)
: preset(preset)
{
	m_presetInformation = make_unique<PresetInformation>(getPresetInformation(preset));
}

void ModelLoadingStage::process()
{
    m_drawablesMap = make_unique<IdDrawablesMap>();
    m_materialMap = make_unique<IdMaterialMap>();
    m_textures = StringTextureMap{};

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropy);

    auto modelFilename = getFilename(preset);
    auto dir = getDirectory(modelFilename);


    const aiScene* assimpScene = aiImportFile(
        modelFilename.c_str(),
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_GenNormals |
        aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials);

    if (!assimpScene)
    {
        std::cout << "Model could not be loaded: " << aiGetErrorString() << std::endl;

        return ;
    }

    for (unsigned int m = 0; m < assimpScene->mNumMaterials; m++)
    {
        auto mat = loadMaterial(assimpScene->mMaterials[m], dir);
        (*m_materialMap)[m] = mat;
        (*m_drawablesMap)[m] = PolygonalDrawables{};
    }

    auto scene = convertScene(assimpScene);
    for (auto mesh : scene->meshes())
    {
        auto& drawables = m_drawablesMap->at(mesh->materialIndex());
        drawables.push_back(make_unique<gloperate::PolygonalDrawable>(*mesh));
    }

    aiReleaseImport(assimpScene);
}

globjects::ref_ptr<globjects::Texture> ModelLoadingStage::loadTexture(const std::string& filename) const
{
    globjects::ref_ptr<globjects::Texture> tex = resourceManager->load<globjects::Texture>(filename);
    tex->setParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);
    tex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    tex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    tex->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    tex->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    tex->setParameter(GL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropy);
    tex->generateMipmap();

    return tex;
}

Material ModelLoadingStage::loadMaterial(aiMaterial* aiMat, const std::string& directory)
{
    Material material;

    for (aiTextureType aiTexType : textureTypes)
    {
        auto type = translateAssimpTextureType(aiTexType);

        float specularFactor;
        aiReturn ret = aiMat->Get(AI_MATKEY_SHININESS, specularFactor);
        if (ret == aiReturn_SUCCESS)
        {
            material.specularFactor = specularFactor;
        }

        aiString texPath;
        ret = aiMat->GetTexture(aiTexType, 0, &texPath);

        if (ret != aiReturn_SUCCESS)
            continue;

        std::string texPathStd = std::string(texPath.C_Str());
        auto path = directory + "/" + texPathStd;
        std::replace(path.begin(), path.end(), '\\', '/');

        globjects::ref_ptr<globjects::Texture> texture = nullptr;
        auto textureIt = m_textures.find(path);
        if (textureIt != m_textures.end())
        {
            texture = (*textureIt).second;
        }
        else
        {
            texture = loadTexture(path);
            m_textures[path] = texture;
        }

        material.addTexture(type, texture);
    }

    return material;
}

PresetInformation ModelLoadingStage::getPresetInformation(Preset preset)
{
    static const std::map<Preset, PresetInformation> conversion {
        //                          camera eye              camera center          near;far         light position          light radius   ground color   ground height  alpha   bump mapping type  reflections  zThickness  focalDist  focalRadius
        { Preset::Imrod,          { { -10.0, 31.2, 10.65 }, { 30, 5.5, -30.0 },    { 0.3, 50000.0 }, { 0, 52, 0 },           1.0f,          { 1, 1, 1 },   0.0f,         1.0f,   BumpType::Normal,  true,        3.0f,       30.0f,     0.003f } },
        { Preset::CrytekSponza,   { { -1300, 250, -23 },    { 0.9, -1.9, -2.1 },   { 5.0, 50000.0 }, { 450, 270, -30 },      15.0f,         { 1, 1, 1 },  -10.0f,        1.0f,   BumpType::Height,  true,        30.0f,      900.0f,    0.003f } },
        { Preset::DabrovicSponza, { { -10.0, 12.6, 0.9 },   { 3.2, 0.28, -1.82 },  { 0.3, 500.0 },   { 0, 18, 0 },           1.0f,          { 1, 1, 1 },   0.0f,         1.0f,   BumpType::Height,  false,       0.0f,       15.0f,     0.003f } },
        { Preset::Jakobi,         { { 0.39, 0.49, -0.63 },  { 0.05, -0.04, -0.1 }, { 0.01, 80.0 },   { -0.4, 1.2, -0.7 },    0.05f,         { 1, 1, 1 },  -0.115f,       1.0f,   BumpType::None,    true,        0.05f,      0.5f,      0.003f } },
        { Preset::Megacity,       { { 0.26, 0.23, -0.35 },  { 0.14, 0.0, -0.14 },  { 0.01, 80.0 },   { -0.4, 1.2, -1.5 },    0.01f,         { 1, 1, 1 },  -0.048f,       1.0f,   BumpType::None,    true,        0.05f,      0.5f,      0.003f } },
        { Preset::Mitusba,        { { 0.2, 3.7, 4.3 },      { 0.16, 0.07, -1.25 }, { 0.3, 600.0 },   { 10, 20, 0 },          0.7f,          { 1, 1, 1 },   0.1f,         0.5f,   BumpType::None,    false,       0.0f,       7.0f,      0.003f } },
        { Preset::Transparency,   { { -1.9, 4.2, 4.6 },     { -0.06, 0.02, 0.56 }, { 0.3, 600.0 },   { 0, 5, 0 },            0.1f,          { 1, 1, 1 },  -1.4f,         0.5f,   BumpType::None,    false,       0.0f,       4.0f,      0.003f } }
    };

    return conversion.at(preset);
}

std::string ModelLoadingStage::getFilename(Preset preset)
{
    static const std::map<Preset, std::string> conversion {
        { Preset::Imrod, "data/Imrod/Imrod.obj" },
        { Preset::CrytekSponza, "data/crytek-sponza/sponza.obj" },
        { Preset::DabrovicSponza, "data/dabrovic-sponza/sponza.obj" },
        { Preset::Jakobi, "data/jakobi/jakobikirchplatz4.obj" },
        { Preset::Megacity, "data/megacity/simple2.obj" },
        { Preset::Mitusba, "data/mitsuba/mitsuba.obj" },
        { Preset::Transparency, "data/transparency_scene.obj" }
    };

    return conversion.at(preset);
}

gloperate::Scene * ModelLoadingStage::convertScene(const aiScene * scene) const
{
    gloperate::Scene * sceneOut = new gloperate::Scene;

    for (size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        sceneOut->meshes().push_back(convertGeometry(scene->mMeshes[i]));
    }

    return sceneOut;
}

gloperate::PolygonalGeometry * ModelLoadingStage::convertGeometry(const aiMesh * mesh) const
{
    gloperate::PolygonalGeometry * geometry = new gloperate::PolygonalGeometry;

    std::vector<unsigned int> indices;
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        const auto & face = mesh->mFaces[i];
        for (auto j = 0u; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    geometry->setIndices(std::move(indices));

    std::vector<glm::vec3> vertices;
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        const auto & vertex = mesh->mVertices[i];
        vertices.push_back({ vertex.x, vertex.y, vertex.z });
    }
    geometry->setVertices(std::move(vertices));

    if (mesh->HasNormals())
    {
        std::vector<glm::vec3> normals;
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            const auto & normal = mesh->mNormals[i];
            normals.push_back({ normal.x, normal.y, normal.z });
        }
        geometry->setNormals(std::move(normals));
    }

    if (mesh->HasTextureCoords(0))
    {
        std::vector<glm::vec3> textureCoordinates;
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            const auto & textureCoordinate = mesh->mTextureCoords[0][i];
            textureCoordinates.push_back({ textureCoordinate.x, textureCoordinate.y, textureCoordinate.z });
        }
        geometry->setTextureCoordinates(std::move(textureCoordinates));
    }

    geometry->setMaterialIndex(mesh->mMaterialIndex);

    return geometry;
}

const PresetInformation& ModelLoadingStage::getCurrentPreset() const
{
    return *m_presetInformation.get();
}
const IdDrawablesMap& ModelLoadingStage::getDrawablesMap() const
{
    return *m_drawablesMap.get();
}
const IdMaterialMap& ModelLoadingStage::getMaterialMap() const
{
    return *m_materialMap.get();
}
