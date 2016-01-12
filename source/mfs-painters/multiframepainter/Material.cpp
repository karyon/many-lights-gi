#include "Material.h"

Material::Material()
: specularFactor(0.0f)
{}

const Material::TextureMap& Material::textureMap() const
{
    return m_textureMap;
}

void Material::addTexture(TextureType type, globjects::ref_ptr<globjects::Texture> texture)
{
    m_textureMap[type] = texture;
}

bool Material::hasTexture(TextureType type) const
{
    return m_textureMap.count(type) > 0;
}
