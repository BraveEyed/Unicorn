/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#ifndef UNICORN_VIDEO_RENDERER_HPP
#define UNICORN_VIDEO_RENDERER_HPP

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>
#include <string>

struct GLFWwindow;

namespace unicorn
{

namespace system
{
class Manager;
class Window;
}

namespace video
{
struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool IsComplete() const
    {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

/** @brief  Vulkan renderer class
 *
 *  Initializes and controls Vulkan API
 */
class Renderer
{
public:
    Renderer(system::Manager& manager,
        system::Window* pWindow);

    ~Renderer();

    Renderer(const Renderer& other) = delete;
    Renderer(const Renderer&& other) = delete;
    Renderer& operator=(const Renderer& other) = delete;
    Renderer& operator=(const Renderer&& other) = delete;

    bool Init();
    void Deinit();
    bool Render();
    bool RecreateSwapChain();

private:
    bool m_isInitialized;

    //! Reference to window manager manager
    system::Manager& m_systemManager;

    //! Pointer to associated window
    system::Window* m_pWindow;

    vk::PhysicalDevice m_vkPhysicalDevice;
    vk::Device m_vkLogicalDevice;
    vk::SwapchainKHR m_vkSwapChain;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    vk::SurfaceKHR m_vkWindowSurface;
    vk::Format m_swapChainImageFormat;
    vk::Extent2D m_swapChainExtent;
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_graphicsPipeline;
    vk::RenderPass m_renderPass;
    vk::CommandPool m_commandPool;
    vk::Semaphore m_imageAvailableSemaphore;
    vk::Semaphore m_renderFinishedSemaphore;
    std::string m_gpuName;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
    std::vector<vk::Framebuffer> m_swapChainFramebuffers;
    std::vector<vk::CommandBuffer> m_commandBuffers;
#ifdef NDEBUG
    static const bool s_enableValidationLayers = false;
#else
    static const bool s_enableValidationLayers = true;
#endif

    void FreeSurface();
    void FreeLogicalDevice();
    void FreeSwapChain();
    void FreeImageViews();
    void FreeRenderPass();
    void FreeGraphicsPipeline();
    void FreeFrameBuffers();
    void FreeCommandPool();
    void FreeCommandBuffers();
    void FreeSemaphores();

    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSurface();
    bool CreateSwapChain();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateGraphicsPipeline();
    bool CreateFramebuffers();
    bool CreateCommandPool();
    bool CreateCommandBuffers();
    bool CreateSemaphores();
    bool CreateShaderModule(const std::vector<uint8_t>& code, vk::ShaderModule& shaderModule);
    bool IsDeviceSuitable(const vk::PhysicalDevice& device);
    bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& device);
    bool Frame();
    QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device);
    bool QuerySwapChainSupport(SwapChainSupportDetails& details, const vk::PhysicalDevice& device);
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    // Callbacks for window events
    void OnWindowDestroyed(system::Window* pWindow);
    void OnWindowSizeChanged(system::Window* pWindow, std::pair<int32_t, int32_t> size);
};
}
}

#endif // UNICORN_VIDEO_RENDERER_HPP
