/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/vulkan/VulkanContext.hpp>
#include <unicorn/Settings.hpp>
#include <unicorn/utility/Logger.hpp>

namespace unicorn
{
namespace video
{
vk::Instance VulkanContext::m_vkInstance = nullptr;
VkDebugReportCallbackEXT VulkanContext::m_vulkanCallback = VK_NULL_HANDLE;
std::vector<const char*> VulkanContext::m_validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
std::vector<const char*> VulkanContext::m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
std::vector<const char*> VulkanContext::m_instanceExtensions;

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,
													  VkDebugReportObjectTypeEXT objType,
													  uint64_t obj,
													  size_t location,
													  int32_t code,
													  const char* layerPrefix,
													  const char* msg,
													  void* userData)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		LOG_ERROR("VULKAN LAYER ERROR: %s", msg);
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOG_WARNING("VULKAN LAYER WARNING: %s", msg);
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		LOG_INFO("VULKAN LAYER PERFORMANCE: %s", msg);
	}
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		LOG_INFO("VULKAN LAYER INFO: %s", msg);
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		LOG_INFO("VULKAN LAYER DEBUG: %s", msg);
	}
	return VK_FALSE;
}

bool VulkanContext::Initialize(system::Manager& manager)
{
	if (s_enableValidationLayers && !CheckValidationLayerSupport())
	{
		LOG_ERROR("Vulkan validation layers requested, but not available!");
		return false;
	}

	const Settings& settings = Settings::Instance();

	vk::ApplicationInfo appInfo(settings.GetApplicationName().c_str(),
		VK_MAKE_VERSION(1, 0, 0),
		settings.GetUnicornEngineName().c_str(),
		VK_MAKE_VERSION(0, 1, 0),
		VK_API_VERSION_1_0);
	vk::InstanceCreateInfo createInfo = {};
	createInfo.pApplicationInfo = &appInfo;

	//TODO: Extensions not working.
	m_instanceExtensions = FillRequiredExtensions(manager);
	//m_deviceExtensions.insert( m_deviceExtensions.end(), requiredExtensions.begin(), requiredExtensions.end() );

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
		LOG_ERROR("Failed to create instance!");
		return false;
	}
	SetupDebugCallback();
	return true;
}

bool VulkanContext::CheckValidationLayerSupport()
{
	vk::Result result;
	std::vector<vk::LayerProperties> availableLayers;
	std::tie(result, availableLayers) = vk::enumerateInstanceLayerProperties();
	if (result != vk::Result::eSuccess)
	{
		LOG_ERROR("Can't enumerate instance layer properties!");
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
			LOG_ERROR("Can't find required Vulkan layers: ");

			for (auto& requiredLayer : m_validationLayers)
			{
				LOG_ERROR("%s", requiredLayer);
			}

			return false;
		}
	}

	LOG_INFO("Picked next Vulkan layers :");

	for (auto& layer : m_validationLayers)
	{
		LOG_INFO("%s", layer);
	}

	return true;
}

void VulkanContext::Destroy()
{
	if (IsInitialized())
	{
		m_vkInstance.destroy();
		m_vkInstance = nullptr;
	}
	FreeDebugCallback();
}

std::vector<const char*> VulkanContext::FillRequiredExtensions(system::Manager& manager)
{
	std::vector<const char*> extensions(manager.GetRequiredVulkanExtensions());

	if (s_enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

bool VulkanContext::DestroyDevice(VulkanDevice* device)
{
	return false;
}

VulkanDevice* VulkanContext::GetNewDevice()
{
	return nullptr;
}


vk::Instance VulkanContext::GetVkInstance() 
	{
	return m_vkInstance;
}

void VulkanContext::SetupDebugCallback() {
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

void VulkanContext::DestroyDebugReportCallbackEXT() {
	auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
			vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugReportCallbackEXT"));

	if (func != nullptr)
	{
		func(m_vkInstance, m_vulkanCallback, nullptr);
	}
}

	VkResult VulkanContext::CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT *pCreateInfo) {
		auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT"));

		if (func != nullptr)
		{
			return func(m_vkInstance, pCreateInfo, nullptr, &m_vulkanCallback);
		}
		else
		{
			LOG_ERROR("Can't setup debug callback'");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}



	bool VulkanContext::IsInitialized() {
		return m_vkInstance;
	}

	void VulkanContext::FreeDebugCallback() {
		if (m_vulkanCallback != VK_NULL_HANDLE)
		{
			DestroyDebugReportCallbackEXT();
		}
	}
	std::vector<const char *> VulkanContext::GetValidationLayers() { return m_validationLayers; }

	std::vector<const char *> VulkanContext::GetRequiredExtensions() {return m_deviceExtensions;}

}
}