/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/

#include <unicorn/video/vulkan/VkTexture.hpp>
#include <unicorn/video/Texture.hpp>
#include <unicorn/utility/Logger.hpp>

namespace unicorn
{
    namespace video
    {
        namespace vulkan
        {
						VkTexture::VkTexture() : m_vkImage(nullptr), m_isInitialized(false)
						{
						}

						const vk::DescriptorImageInfo & VkTexture::GetDescriptorImageInfo()
						{
								return m_imageInfo;
						}

						bool VkTexture::Create(const vk::PhysicalDevice & physicalDevice, const vk::Device & device, const vk::CommandPool & commandPool, const vk::Queue & queue, const Texture * texture)
						{
								if (!m_isInitialized)
								{
										Buffer imageStagingBuffer;
										bool result = imageStagingBuffer.Create(physicalDevice, device,
												vk::BufferUsageFlagBits::eTransferSrc,
												vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
												texture->Size());
										if (!result)
										{
												LOG_ERROR("Can't allocate staging buffer for texture - ", texture->Path());
												return false;
										}
										imageStagingBuffer.Map();
										imageStagingBuffer.Write(texture->Data());
										imageStagingBuffer.Unmap();

										m_vkImage = new Image(physicalDevice,
												device,
												vk::Format::eR8G8B8A8Unorm,
												vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
												texture->Width(),
												texture->Height());
										if (!m_vkImage->IsInitialized())
										{
												LOG_ERROR("Can't allocate vulkan based image for texture - ", texture->Path());
												return false;
										}

										m_vkImage->TransitionLayout(vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal, commandPool, queue);
										imageStagingBuffer.CopyToImage(*m_vkImage, commandPool, queue);
										m_vkImage->TransitionLayout(vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, commandPool, queue);

										imageStagingBuffer.Destroy();
										//m_imageInfo.imageView = textureImageView;
										//m_imageInfo.sampler = textureSampler;
										m_isInitialized = true;
								}
						}
				}
    }
}
