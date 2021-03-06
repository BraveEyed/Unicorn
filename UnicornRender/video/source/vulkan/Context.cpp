/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/vulkan/Context.hpp>
#include <unicorn/utility/Settings.hpp>

#include <unicorn/utility/InternalLoggers.hpp>

namespace unicorn
{
namespace video
{
namespace vulkan
{
Context::Context() : m_validationLayers({"VK_LAYER_LUNARG_standard_validation"})
                   , m_deviceExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
                   , m_vkInstance(nullptr)
                   , m_vulkanCallback(NULL)
{
}

#ifdef NDEBUG
const bool Context::s_enableValidationLayers = false;
#else
const bool Context::s_enableValidationLayers = true;
#endif

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,
                                                      VkDebugReportObjectTypeEXT /*objType*/,
                                                      uint64_t /*obj*/,
                                                      size_t /*location*/,
                                                      int32_t /*code*/,
                                                      const char* /*layerPrefix*/,
                                                      const char* msg,
                                                      void* /*userData*/)
{
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        LOG_VULKAN->Error("VULKAN LAYER ERROR: {}", msg);
    }
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        LOG_VULKAN->Warning("VULKAN LAYER WARNING: {}", msg);
    }
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        LOG_VULKAN->Info("VULKAN LAYER PERFORMANCE: {}", msg);
    }
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        LOG_VULKAN->Info("VULKAN LAYER INFO: {}", msg);
    }
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        LOG_VULKAN->Info("VULKAN LAYER DEBUG: {}", msg);
    }
    return VK_FALSE;
}

bool Context::Initialize(system::Manager& manager)
{
    if (s_enableValidationLayers && !CheckValidationLayerSupport())
    {
        LOG_VULKAN->Error("Vulkan validation layers requested, but not available!");
        return false;
    }

    const utility::Settings& settings = utility::Settings::Instance();

    vk::ApplicationInfo appInfo(settings.GetApplicationName().c_str(),
                                VK_MAKE_VERSION(1, 0, 0),
                                settings.GetUnicornEngineName().c_str(),
                                VK_MAKE_VERSION(0, 1, 0),
                                VK_API_VERSION_1_0);
    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;

    m_instanceExtensions = FillRequiredExtensions(manager);

    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_instanceExtensions.data();

    if (s_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vk::createInstance(&createInfo, nullptr, &m_vkInstance) != vk::Result::eSuccess)
    {
        LOG_VULKAN->Error("Failed to create instance!");
        return false;
    }
    SetupDebugCallback();
    return true;
}

bool Context::CheckValidationLayerSupport()
{
    vk::Result result;
    std::vector<vk::LayerProperties> availableLayers;
    std::tie(result, availableLayers) = vk::enumerateInstanceLayerProperties();
    if (result != vk::Result::eSuccess)
    {
        LOG_VULKAN->Error("Can't enumerate instance layer properties!");
        return false;
    }
    for (const char* layerName : m_validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            LOG_VULKAN->Error("Can't find required Vulkan layers: ");

            for (auto& requiredLayer : m_validationLayers)
            {
                LOG_VULKAN->Error("{}", requiredLayer);
            }

            return false;
        }
    }

    LOG_VULKAN->Info("Picked next Vulkan layers :");

    for (auto& layer : m_validationLayers)
    {
        LOG_VULKAN->Info("{}", layer);
    }

    return true;
}

void Context::Deinitialize()
{
    FreeDebugCallback();
    if (IsInitialized())
    {
        m_vkInstance.destroy();
        m_vkInstance = nullptr;
    }
}

std::vector<const char*> Context::FillRequiredExtensions(system::Manager& manager)
{
    std::vector<const char*> extensions(manager.GetRequiredVulkanExtensions());

    if (s_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

vk::Instance Context::GetVkInstance()
{
    return m_vkInstance;
}

const std::vector<const char*>& Context::GetValidationLayers()
{
    return m_validationLayers;
}

const std::vector<const char*>& Context::GetDeviceExtensions()
{
    return m_deviceExtensions;
}

const std::vector<const char*>& Context::GetInstanceExtensions()
{
    return m_instanceExtensions;
}

void Context::SetupDebugCallback()
{
    if (!s_enableValidationLayers)
    {
        return;
    }

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = DebugCallback;
    CreateDebugReportCallbackEXT(&createInfo);
}

void Context::DestroyDebugReportCallbackEXT()
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugReportCallbackEXT"));

    if (func != nullptr)
    {
        func(m_vkInstance, m_vulkanCallback, nullptr);
    }
}

VkResult Context::CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT"));

    if (func != nullptr)
    {
        return func(m_vkInstance, pCreateInfo, nullptr, &m_vulkanCallback);
    }
    else
    {
        LOG_VULKAN->Error("Can't setup debug callback'");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

bool Context::IsInitialized()
{
    return m_vkInstance;
}

void Context::FreeDebugCallback()
{
    if (m_vulkanCallback != NULL)
    {
        DestroyDebugReportCallbackEXT();
    }
}
}
}
}
