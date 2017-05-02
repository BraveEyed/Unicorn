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
Renderer::Renderer(system::Manager& manager, system::Window* window)
    : m_isInitialized(false), m_systemManager(manager), m_pWindow(window), m_pCamera(new Camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}))
{
    m_pCamera->SetPerspective(45.0f, static_cast<float>(window->GetSize().first) / window->GetSize().second, 0.1f, 100.0f);
}

Renderer::~Renderer()
{
}

Camera* Renderer::GetCamera() const
{
    return m_pCamera;
}
}
}