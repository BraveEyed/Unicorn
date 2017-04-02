/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/window_manager/profilers/WindowProfiler.hpp>

#include <unicorn/window_manager/Hub.hpp>
#include <unicorn/window_manager/Window.hpp>

#include <unicorn/utility/Logger.hpp>

namespace unicorn
{
namespace WindowManager
{

WindowProfiler::WindowProfiler(Hub& hub)
    : m_hub( hub )
{
    LOG_INFO("WindowProfiler created");
    m_hub.WindowCreated.connect(this, &WindowProfiler::OnWindowCreated);
}

WindowProfiler::~WindowProfiler()
{
    m_hub.WindowCreated.disconnect(this, &WindowProfiler::OnWindowCreated);

    LOG_INFO("WindowProfiler destroyed");
}

void WindowProfiler::OnWindowCreated(Window* pWindow)
{
    LOG_INFO("Window[%d]: created", pWindow->GetId());

    pWindow->Destroyed.connect(this, &WindowProfiler::OnWindowDestroyed);

    pWindow->PositionChanged.connect(this, &WindowProfiler::OnWindowPositionChanged);
    pWindow->SizeChanged.connect(this, &WindowProfiler::OnWindowSizeChanged);
    pWindow->Close.connect(this, &WindowProfiler::OnWindowClose);
    pWindow->ContentRefresh.connect(this, &WindowProfiler::OnWindowContentRefresh);
    pWindow->Focused.connect(this, &WindowProfiler::OnWindowFocused);
    pWindow->Minimized.connect(this, &WindowProfiler::OnWindowMinimized);
    pWindow->Maximized.connect(this, &WindowProfiler::OnWindowMaximized);
    pWindow->FramebufferResized.connect(this, &WindowProfiler::OnWindowFramebufferResized);
}

void WindowProfiler::OnWindowDestroyed(Window* pWindow)
{
    LOG_INFO("Window[%d]: destroyed", pWindow->GetId());
}

void WindowProfiler::OnWindowPositionChanged(Window* pWindow, std::pair<int32_t, int32_t> position)
{
    LOG_INFO("Window[%d]: position changed to %d:%d", pWindow->GetId(), position.first, position.second);
}

void WindowProfiler::OnWindowSizeChanged(Window* pWindow, std::pair<int32_t, int32_t> size)
{
    LOG_INFO("Window[%d]: size changed to %d:%d", pWindow->GetId(), size.first, size.second);
}

void WindowProfiler::OnWindowClose(Window* pWindow)
{
    LOG_INFO("Window[%d]: closed", pWindow->GetId());
}

void WindowProfiler::OnWindowContentRefresh(Window* pWindow)
{
    LOG_INFO("Window[%d]: content refresh requested", pWindow->GetId());
}

void WindowProfiler::OnWindowFocused(Window* pWindow, bool flag)
{
    LOG_INFO("Window[%d]: focus %s", pWindow->GetId(), flag ? "gained" : "lost");
}

void WindowProfiler::OnWindowMinimized(Window* pWindow, bool flag)
{
    LOG_INFO("Window[%d]: minimize event: %s", pWindow->GetId(), flag ? "iconify" : "restore");
}

void WindowProfiler::OnWindowMaximized(Window* pWindow, bool flag)
{
    LOG_INFO("Window[%d]: maximize event: %s", pWindow->GetId(), flag ? "maximize" : "restore");
}

void WindowProfiler::OnWindowFramebufferResized(Window* pWindow, std::pair<int32_t, int32_t> size)
{
    LOG_INFO("Window[%d]: framebuffer size changed to %d:%d", pWindow->GetId(), size.first, size.second);
}


}
}
