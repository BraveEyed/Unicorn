/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/Material.hpp>
#include <unicorn/video/Texture.hpp>
#include <unicorn/utility/Logger.hpp>

namespace unicorn
{
namespace video
{
Material::Material()
    : m_color(Color::Red())
    , m_spriteCoordinates(0, 0, 1, 1)
    , m_isColored(true)
    , m_isWired(false)
    , m_isVisible(true)
    , m_albedo(nullptr)
{
}

void Material::SetAlbedo(std::shared_ptr<Texture> albedo)
{
    if(albedo == nullptr || !albedo->IsLoaded())
    {
        LOG_ERROR("Setting not loaded texture to material!");
        return;
    }
    m_isColored = false;
    m_albedo = albedo;

    DataUpdated.emit();
}

void Material::SetIsWired(bool wireframe)
{
    m_isWired = wireframe;

    DataUpdated.emit();
}

void Material::SetIsColored(bool colored)
{
    m_isColored = colored;

    DataUpdated.emit();
}

void Material::RemoveAlbedo()
{
    SetAlbedo(nullptr);
    SetIsColored(true);
}

bool Material::IsColored() const
{
    return m_isColored;
}

bool Material::IsWired() const
{
    return m_isWired;
}

void Material::SetIsVisible(bool visible)
{
    m_isVisible = visible;

    DataUpdated.emit();
}

bool Material::IsVisible() const
{
    return m_isVisible;
}

std::shared_ptr<Texture> Material::GetAlbedo() const
{
    return m_albedo;
}

void Material::SetColor(glm::vec3 color)
{
    m_color = color;

    DataUpdated.emit();
}

glm::vec3 Material::GetColor() const
{
    return m_color;
}

void Material::SetSpriteCoordinates(int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (!m_albedo->IsLoaded())
    {
        LOG_WARNING("Setting sprite coordinates, when texture was not set, nothing will be set.");
        return;
    }

    m_spriteCoordinates = { static_cast<float>(x) / m_albedo->Width(),
        static_cast<float>(y) / m_albedo->Height(),
        static_cast<float>(width) / m_albedo->Width(),
        static_cast<float>(height) / m_albedo->Height() };

    DataUpdated.emit();
}

void Material::SetNormalizedSpriteCoordinates(float x, float y, float width, float height)
{
    if (!m_albedo->IsLoaded())
    {
        LOG_WARNING("Setting sprite coordinates, when texture was not set, nothing will be set.");
        return;
    }

    m_spriteCoordinates = { x, y, width, height };

    DataUpdated.emit();
}

glm::vec4 Material::GetSpriteCoordinates() const
{
    if(m_albedo == nullptr)
    {
        LOG_WARNING("Asks for sprite coordinates without albedo texture!");
        return GetNormalizedSpriteCoordinates();
    }

    glm::vec4 const denormalized = { m_spriteCoordinates.x * m_albedo->Width(),
        m_spriteCoordinates.y * m_albedo->Height(),
        m_spriteCoordinates.z * m_albedo->Width(),
        m_spriteCoordinates.w * m_albedo->Height() };
    return denormalized;
}

glm::vec4 Material::GetNormalizedSpriteCoordinates() const
{
    return m_spriteCoordinates;
}

}
}
