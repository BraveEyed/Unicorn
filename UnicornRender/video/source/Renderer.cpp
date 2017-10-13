/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/Renderer.hpp>
#include <unicorn/system/Window.hpp>
#include <unicorn/video/Camera.hpp>

namespace unicorn
{
namespace video
{
Renderer::Renderer(system::Manager& manager, system::Window* window, Camera& camera)
    : camera(&camera)
    , m_isInitialized(false)
    , m_systemManager(manager)
    , m_pWindow(window)
    , m_backgroundColor({ {0.0f, 0.0f, 0.0f, 0.0f} })
    , m_depthTestEnabled(true)
{
}

Renderer::~Renderer()
{
    Destroyed.emit(this);
    Destroyed.clear();
}

void Renderer::SetBackgroundColor(const glm::vec3& backgroundColor)
{
    m_backgroundColor[0] = backgroundColor.r;
    m_backgroundColor[1] = backgroundColor.g;
    m_backgroundColor[2] = backgroundColor.b;
    m_backgroundColor[3] = 1.0f;
}
}
}
