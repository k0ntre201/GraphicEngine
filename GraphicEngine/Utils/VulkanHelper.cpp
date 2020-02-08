#include "VulkanHelper.hpp"
#include <iomanip>
#include <iostream>

#undef max

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (pCallbackData != nullptr)
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

std::vector<const char*> GraphicEngine::Utils::Vulkan::getDeviceExtension()
{
	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	return deviceExtensions;
}

std::optional<uint32_t> GraphicEngine::Utils::Vulkan::getGraphicQueueFamilyIndex(const vk::PhysicalDevice& physicalDevice)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	uint32_t tempGraphicQueueFamilyIndex = std::distance(std::begin(queueFamilyProperties),
		std::find_if(std::begin(queueFamilyProperties), std::end(queueFamilyProperties),
			[](vk::QueueFamilyProperties prop) {return prop.queueFlags & vk::QueueFlagBits::eGraphics; }));

	return tempGraphicQueueFamilyIndex < queueFamilyProperties.size() ? std::optional<uint32_t>{tempGraphicQueueFamilyIndex} : std::nullopt;
}

GraphicEngine::Utils::Vulkan::QueueFamilyIndices GraphicEngine::Utils::Vulkan::findGraphicAndPresentQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

	QueueFamilyIndices indices;

	indices.graphicsFamily = getGraphicQueueFamilyIndex(physicalDevice);

	if (indices.graphicsFamily.has_value())
	{
		indices.presentFamily = physicalDevice.getSurfaceSupportKHR(indices.graphicsFamily.value(), surface.get()) == VK_TRUE ? indices.graphicsFamily : std::nullopt;
		if (!indices.presentFamily.has_value())
		{
			uint32_t tempGraphicQueueFamilyIndex = std::distance(std::begin(queueFamilyProperties),
				std::find_if(std::begin(queueFamilyProperties), std::end(queueFamilyProperties),
					[&](vk::QueueFamilyProperties prop)
					{
						return prop.queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(indices.graphicsFamily.value(), surface.get());
					}));

			if (tempGraphicQueueFamilyIndex < queueFamilyProperties.size())
			{
				indices.graphicsFamily = tempGraphicQueueFamilyIndex;
				indices.presentFamily = tempGraphicQueueFamilyIndex;
			}

			else
			{
				for (size_t i{ 0 }; i < queueFamilyProperties.size(); ++i)
				{
					if (physicalDevice.getSurfaceSupportKHR(i, surface.get()))
					{
						indices.presentFamily = i;
						break;
					}
				}
			}
		}
	}
	return indices;
}

vk::UniqueInstance GraphicEngine::Utils::Vulkan::createUniqueInstance(std::string appName, std::string engineName, std::vector<std::string> validationLayers, std::vector<std::string> extensionLayers, uint32_t apiVersion)
{
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
	static vk::DynamicLoader dl;
#endif

#if !defined(NDEBUG)
	auto layerProperties = vk::enumerateInstanceLayerProperties();
	auto extensionProperties = vk::enumerateInstanceExtensionProperties();
#endif

	std::vector<const char*> requiredLayers;

	for (auto const& layer : validationLayers)
	{
		if (std::find_if(std::begin(layerProperties), std::end(layerProperties), [layer](auto layerProperty) { return layer == layerProperty.layerName; }) == std::end(layerProperties))
		{
			throw std::runtime_error("Failed to match validation layers!");
		}
		requiredLayers.push_back(layer.c_str());
	}

	std::vector<const char*> extensions;
	for (auto const& extension : extensionLayers)
	{
		if (std::find_if(std::begin(extensionProperties), std::end(extensionProperties), [extension](auto extensionProperty) { return extension == extensionProperty.extensionName; }) == std::end(extensionProperties))
		{
			throw std::runtime_error("Failed to match extension layers!");
		}
		extensions.push_back(extension.c_str());
	}

#if !defined(NDEBUG)
	if (std::find(std::begin(extensionLayers), std::end(extensionLayers), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == std::end(extensionLayers))
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif
	vk::ApplicationInfo appInfo(appName.c_str(), VK_MAKE_VERSION(1, 0, 0), engineName.c_str(), VK_MAKE_VERSION(1, 0, 0), apiVersion);

#if defined(NDEBUG)
	vk::StructureChain<vk::InstanceCreateInfo> createInfo({ {}, &appInfo, requiredLayers.size(), requiredLayers.data(), extensions.size(), extensions.data() });
#else
	vk::DebugUtilsMessageSeverityFlagsEXT severityFlag(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose);

	vk::DebugUtilsMessageTypeFlagsEXT typeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

	vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> createInfo(
		{ {}, &appInfo, static_cast<uint32_t>(requiredLayers.size()), requiredLayers.data(), static_cast<uint32_t>(extensions.size()), extensions.data() },
		{ {}, severityFlag, typeFlags, debugCallback });
#endif // !DEBUG

	vk::UniqueInstance instance = vk::createInstanceUnique(createInfo.get<vk::InstanceCreateInfo>());

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
	VULKAN_HPP_DISPATCH_LOADER_DYNAMIC.init(*instance);
#endif

	return instance;
}

bool GraphicEngine::Utils::Vulkan::isPhysicalDeviceSituable(const vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface)
{
	QueueFamilyIndices indices = findGraphicAndPresentQueueFamilyIndices(physicalDevice, surface);

	return indices.isComplete() && physicalDevice.getFeatures().samplerAnisotropy;;
}

vk::PhysicalDevice GraphicEngine::Utils::Vulkan::getPhysicalDevice(const vk::UniqueInstance& instance, vk::UniqueSurfaceKHR& surface)
{
	auto physicalDevices = instance->enumeratePhysicalDevices();

	for (auto physicalDevice : physicalDevices)
	{
		if (isPhysicalDeviceSituable(physicalDevice, surface))
		{
			return physicalDevice;
		}
	}

	return vk::PhysicalDevice();
}

vk::UniqueDevice GraphicEngine::Utils::Vulkan::getUniqueLogicalDevice(const vk::PhysicalDevice& physicalDevice, vk::UniqueSurfaceKHR& surface)
{
	QueueFamilyIndices indices = findGraphicAndPresentQueueFamilyIndices(physicalDevice, surface);

	if (indices.isComplete())
	{
		std::set<uint32_t> setIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
		for (uint32_t ind : setIndices)
		{
			float queuePriority{ 1.0f };
			deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), ind, 1, &queuePriority));
		}

		vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();

		auto extensions = getDeviceExtension();
		vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
			static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(),
			0, nullptr,
			static_cast<uint32_t>(extensions.size()), extensions.data(),
			&deviceFeatures);


		return physicalDevice.createDeviceUnique(deviceCreateInfo);
	}

	throw std::runtime_error("Failed to create logical device!");
}

vk::UniqueCommandPool GraphicEngine::Utils::Vulkan::createUniqueCommandPool(const vk::UniqueDevice& device, const QueueFamilyIndices& queueFamilyIndex)
{
	if (queueFamilyIndex.graphicsFamily.has_value())
	{
		vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex.graphicsFamily.value());
		return device->createCommandPoolUnique(createInfo);
	}

	throw std::runtime_error("Graphic queue family index is empty!");
}

std::vector<vk::UniqueCommandBuffer> GraphicEngine::Utils::Vulkan::createUniqueCommandBuffers(const vk::UniqueDevice& device, const vk::UniqueCommandPool& commandPool)
{
	//vk::CommandBufferAllocateInfo allocateInfo(commandPool.get(),vk::CommandBufferLevel::ePrimary, )
	return std::vector<vk::UniqueCommandBuffer>();
}

uint32_t GraphicEngine::Utils::Vulkan::findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags memoryProperty)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

	for (int i{ 0 }; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & (1 << i) && memoryProperties.memoryTypes[i].propertyFlags == memoryProperty)
			return i;
	}

	throw std::runtime_error("Failed to find situable memory!");
}

vk::UniqueDeviceMemory GraphicEngine::Utils::Vulkan::allocateMemory(const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, vk::MemoryPropertyFlags memoryProperty, const vk::MemoryRequirements& memoryRequirements)
{
	return device->allocateMemoryUnique(vk::MemoryAllocateInfo(memoryRequirements.size, findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperty)));
}

GraphicEngine::Utils::Vulkan::SwapChainData::SwapChainData(const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, const vk::UniqueSurfaceKHR& surface, const QueueFamilyIndices& indices, const vk::Extent2D& extend, const vk::UniqueSwapchainKHR& oldSwapChain, vk::ImageUsageFlags imageUsage)
{
	createSwapChainData(physicalDevice, device, surface, indices, extend, oldSwapChain, imageUsage);
}

void GraphicEngine::Utils::Vulkan::SwapChainData::createSwapChainData(const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, const vk::UniqueSurfaceKHR& surface, const QueueFamilyIndices& indices, const vk::Extent2D& extend, const vk::UniqueSwapchainKHR& oldSwapChain, vk::ImageUsageFlags imageUsage)
{
	SwapChainSupportDetails swapChainSupportDetails = getSwapChainSuppordDetails(physicalDevice, surface);
	this->extent = pickSurfaceExtent(swapChainSupportDetails.capabilities, extend);
	vk::SurfaceFormatKHR surfaceFormat = pickSurfaceFormat(swapChainSupportDetails.formats).format;
	this->format = surfaceFormat.format;

	uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
	if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
	{
		imageCount = swapChainSupportDetails.capabilities.maxImageCount;
	}
	vk::SurfaceTransformFlagBitsKHR preTransform = (swapChainSupportDetails.capabilities.currentTransform & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity : swapChainSupportDetails.capabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR compositeAlpha =
		(swapChainSupportDetails.capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied :
		(swapChainSupportDetails.capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied :
		(swapChainSupportDetails.capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit : vk::CompositeAlphaFlagBitsKHR::eOpaque;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, surface.get(), imageCount, surfaceFormat.format, surfaceFormat.colorSpace, this->extent, 1, imageUsage, vk::SharingMode::eExclusive, 0, nullptr,
		preTransform, compositeAlpha, pickPresentMode(swapChainSupportDetails.presentModes), true, *oldSwapChain);

	swapChain = device->createSwapchainKHRUnique(swapChainCreateInfo);
	images = device->getSwapchainImagesKHR(swapChain.get());


	vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
	vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	for (auto image : images)
	{
		vk::ImageViewCreateInfo createInfo(vk::ImageViewCreateFlags() , image, vk::ImageViewType::e2D, this->format, componentMapping, subResourceRange);
		imageViews.push_back(device->createImageViewUnique(createInfo));
	}
}

GraphicEngine::Utils::Vulkan::SwapChainSupportDetails GraphicEngine::Utils::Vulkan::SwapChainData::getSwapChainSuppordDetails(const vk::PhysicalDevice& physicalDevice, const vk::UniqueSurfaceKHR& surface)
{
	return SwapChainSupportDetails(physicalDevice.getSurfaceCapabilitiesKHR(surface.get()), physicalDevice.getSurfaceFormatsKHR(surface.get()), physicalDevice.getSurfacePresentModesKHR(surface.get()));
}

vk::SurfaceFormatKHR GraphicEngine::Utils::Vulkan::SwapChainData::pickSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
	if (formats.size() > 0)
	{
		if (formats[0].format == vk::Format::eUndefined)
		{
			return vk::SurfaceFormatKHR(vk::Format::eB8G8R8Snorm);
		}

		std::array<vk::Format, 4> requestFormats = { vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm };

		for (const vk::Format requestFormat : requestFormats)
		{
			auto it = std::find_if(std::begin(formats), std::end(formats),
				[requestFormat](vk::SurfaceFormatKHR format) { return format.format == requestFormat && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
			if (it != std::end(formats));
			{
				return *it;
			}
		}
	}

	throw std::runtime_error("Failed to pick surface format!");
}

vk::PresentModeKHR GraphicEngine::Utils::Vulkan::SwapChainData::pickPresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
	if (presentModes.size() > 0)
	{
		auto it = std::find(std::begin(presentModes), std::end(presentModes), vk::PresentModeKHR::eMailbox);
		if (it != std::end(presentModes))
		{
			return *it;
		}

		it = std::find(std::begin(presentModes), std::end(presentModes), vk::PresentModeKHR::eImmediate);
		if (it != std::end(presentModes))
		{
			return *it;
		}
	}

	throw std::runtime_error("Failed to pick present mode!");
}

vk::Extent2D GraphicEngine::Utils::Vulkan::SwapChainData::pickSurfaceExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D frameBufferExtent)
{
	if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		vk::Extent2D extent;
		extent.width = std::clamp(frameBufferExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(frameBufferExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return extent;
	}

	return capabilities.currentExtent;
}

GraphicEngine::Utils::Vulkan::SwapChainSupportDetails::SwapChainSupportDetails()
{
}

GraphicEngine::Utils::Vulkan::SwapChainSupportDetails::SwapChainSupportDetails(vk::SurfaceCapabilitiesKHR capabilities, std::vector<vk::SurfaceFormatKHR> formats, std::vector<vk::PresentModeKHR> presentModes) :
	capabilities(capabilities), formats(formats), presentModes(presentModes)
{
}

GraphicEngine::Utils::Vulkan::ImageData::ImageData(const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, vk::Extent3D extent, vk::Format format, vk::SampleCountFlagBits numOfSamples,
	vk::MemoryPropertyFlags memoryProperty, vk::ImageUsageFlags imageUsage, vk::ImageTiling tiling,
	uint32_t mipLevel, vk::ImageLayout layout, vk::ImageAspectFlags aspectFlags)
{
	this->format = format;

	vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(), vk::ImageType::e2D, format, extent, mipLevel, 1, numOfSamples, tiling, imageUsage, vk::SharingMode::eExclusive, 0, nullptr, layout);

	this->image = device->createImageUnique(imageCreateInfo);
	this->deviceMemory = allocateMemory(physicalDevice, device, memoryProperty, device->getImageMemoryRequirements(image.get()));

	device->bindImageMemory(image.get(), deviceMemory.get(), 0);

	vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
	vk::ImageSubresourceRange subResourceRange(aspectFlags, 0, 1, 0, 1);
	vk::ImageViewCreateInfo createInfo(vk::ImageViewCreateFlags(), image.get(), vk::ImageViewType::e2D, this->format, componentMapping, subResourceRange);
	imageView = device->createImageViewUnique(createInfo);
}

GraphicEngine::Utils::Vulkan::DeepBufferData::DeepBufferData(const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, vk::Extent3D extent, vk::Format format, vk::SampleCountFlagBits numOfSamples) :
	ImageData(physicalDevice, device, extent, format, numOfSamples,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, 1, vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eDepth)
{
}
