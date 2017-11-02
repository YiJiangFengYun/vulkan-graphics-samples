#include "graphics/app/app.hpp"

namespace kgs
{
	bool checkDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice, std::vector<const char*> deviceExtensionNames)
	{
		auto extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensionNames(deviceExtensionNames.begin(), deviceExtensionNames.end());

		for (const auto& extPro : extensionProperties)
		{
			requiredExtensionNames.erase(extPro.extensionName);
		}

		return requiredExtensionNames.empty();
	}

	Application::Application(std::string name, uint32_t version)
		: m_appName(name)
		, m_appVersion(version)
		, m_engineName("No engine")
		, m_engineVersion(VK_MAKE_VERSION(1, 0, 0))
	{

	}

	Application::~Application()
	{
#ifdef DEBUG
		if (m_debugReportCallBack != vk::DebugReportCallbackEXT(nullptr))
		{
			destroyDebugReportCallbackEXT(*m_pInstance, m_debugReportCallBack, nullptr);
		}
#endif // DEBUG
	}

	void Application::initCreateVkInstance()
	{
#ifdef ENABLE_VALIDATION_LAYERS
		if (_checkValidationLayerSupport() == false)
		{
			throw std::runtime_error("Validation layers requested is not available!");
		}
#endif // ENABLE_VALIDATION_LAYERS
		vk::ApplicationInfo appInfo = { m_appName.data(), m_appVersion, m_engineName, m_engineVersion, VK_API_VERSION_1_0 };

		//query application required extensions.
		auto requiredExtensions = _getRequiredExtensions();

#ifdef DEBUG
		//query available extensions.
		auto availableExtensions = vk::enumerateInstanceExtensionProperties();

		//print available extensions.
		LOG(plog::debug) << "Available extensions: " << std::endl;
		for (const auto& extension : availableExtensions)
		{
			LOG(plog::debug) << "\t" << extension.extensionName << std::endl;
		}
#endif // DEBUG

#ifdef ENABLE_VALIDATION_LAYERS
		vk::InstanceCreateInfo createInfo = {
			vk::InstanceCreateFlags(),
			&appInfo,
			static_cast<uint32_t>(validationlayers.size()),
			validationlayers.data(),
			static_cast<uint32_t>(requiredExtensions.size()),
			requiredExtensions.data()
		};
#else
		vk::InstanceCreateInfo createInfo = {
			vk::InstanceCreateFlags(),
			&appInfo,
			0,
			nullptr,
			static_cast<uint32_t>(requiredExtensions.size()),
			requiredExtensions.data()
		};
#endif // ENABLE_VALIDATION_LAYERS

		//create vulkan instance
		m_pInstance = fd::createInstance(createInfo);

#ifdef DEBUG
		_setupDebugCallBack();
#endif // DEBUG
	}

	void Application::initOther(std::shared_ptr<vk::SurfaceKHR> pSurface)
	{
		_pickPhysicalDevice(pSurface);
		_createLogicDevice(pSurface);
	}

	std::shared_ptr<vk::Instance> Application::getVKInstance()
	{
		return m_pInstance;
	}

	std::shared_ptr<vk::PhysicalDevice> Application::getPhysicalDevice()
	{
		return m_pPhysicalDevice;
	}

	std::shared_ptr<vk::Device> Application::getDevice()
	{
		return m_pDevice;
	}

	vk::Queue Application::getGraphicsQueue()
	{
		return m_graphicsQueue;
	}

	vk::Queue Application::getPresentQueue()
	{
		return m_presentQueue;
	}

#ifdef ENABLE_VALIDATION_LAYERS
	bool Application::_checkValidationLayerSupport()
	{
		//query available layers.
		auto avaibleLayers = vk::enumerateInstanceLayerProperties();

		//print available layers.
		LOG(plog::debug) << "Available layers: " << std::endl;
		for (const auto& layerProperties : avaibleLayers)
		{
			LOG(plog::debug) << "\t" << layerProperties.layerName << std::endl;
		}

		for (const auto& layerName : validationlayers)
		{
			bool layerFound = false;
			for (const auto& layerProperties : avaibleLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (layerFound == false)
			{
				return false;
			}
		}

		return true;
	}
#endif // ENABLE_VALIDATION_LAYERS

	std::vector<const char*> Application::_getRequiredExtensions()
	{
		std::vector<const char*> requiredExtensions;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
		uint32_t requiredExtensionCount = glfwExtensionCount + 1;
#else
		uint32_t requiredExtensionCount = glfwExtensionCount;
#endif // ENABLE_VALIDATION_LAYERS

		requiredExtensions.resize(requiredExtensionCount);

		for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		{
			requiredExtensions[i] = glfwExtensions[i];
		}

#ifdef ENABLE_VALIDATION_LAYERS
		requiredExtensions[glfwExtensionCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
#endif // ENABLE_VALIDATION_LAYERS

		return requiredExtensions;
	}

#ifdef DEBUG
	void Application::_setupDebugCallBack()
	{
		vk::DebugReportCallbackCreateInfoEXT createInfo = {
			vk::DebugReportFlagBitsEXT::eDebug |
			vk::DebugReportFlagBitsEXT::eInformation |
			vk::DebugReportFlagBitsEXT::eWarning |
			vk::DebugReportFlagBitsEXT::ePerformanceWarning |
			vk::DebugReportFlagBitsEXT::eError,
			debugCallback,
			this
		};

		VkDebugReportCallbackEXT callback;
		createDebugReportCallbackEXT(*m_pInstance, &VkDebugReportCallbackCreateInfoEXT(createInfo),
			nullptr, &callback);
		m_debugReportCallBack = callback;
	}

	void Application::_onDebugCallBack(vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code, const char* layerPrefix,
		const char* msg)
	{
		if (flags & vk::DebugReportFlagBitsEXT::eDebug)
		{
			LOG(plog::debug) << code << " : " << msg << " at " << layerPrefix << std::endl;
		}
		else if (flags & vk::DebugReportFlagBitsEXT::eInformation)
		{
			LOG(plog::info) << code << " : " << msg << " at " << layerPrefix << std::endl;
		}
		else if (flags & vk::DebugReportFlagBitsEXT::eWarning)
		{
			LOG(plog::warning) << code << " : " << msg << " at " << layerPrefix << std::endl;
		}
		else if (flags & vk::DebugReportFlagBitsEXT::ePerformanceWarning)
		{
			LOG(plog::warning) << code << " : " << msg << " at " << layerPrefix << std::endl;
		}
		else if (flags & vk::DebugReportFlagBitsEXT::eError)
		{
			LOG(plog::error) << code << " : " << msg << " at " << layerPrefix << std::endl;
			throw std::runtime_error(std::to_string(code) + " : " + std::string(msg) + " at " + std::string(layerPrefix));
		}
		else
		{
			LOG(plog::info) << code << " : " << msg << " at " << layerPrefix << std::endl;
		}
	}
#endif // DEBUG

#ifdef DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void * userData)
	{
		Application* context = (Application *)userData;
		context->_onDebugCallBack(vk::DebugReportFlagsEXT(vk::DebugReportFlagBitsEXT(flags)),
			vk::DebugReportObjectTypeEXT(objType),
			obj, location,
			code,
			layerPrefix,
			msg);

		return VK_FALSE;
	}
#endif // DEBUG

	void Application::_pickPhysicalDevice(std::shared_ptr<vk::SurfaceKHR> pSurface)
	{
		auto physicalDevices = m_pInstance->enumeratePhysicalDevices();

		LOG(plog::debug) << "physical device num: " + physicalDevices.size() << std::endl;

		if (physicalDevices.size() == 0)
		{
			throw std::runtime_error("Failed to find GPUs with VULKAN support.");
		}

		const size_t size = physicalDevices.size();

		for (auto it = physicalDevices.cbegin(); it != physicalDevices.cend();)
		{
			Bool32 isSuitable = [&](const vk::PhysicalDevice& physicalDevice)->Bool32
			{
				const vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
				const vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();
				//Application can't function without geometry shaders
				if (deviceFeatures.geometryShader == VK_FALSE)
				{
					return KGS_FALSE;
				}

				//Application can't function without queue family that supports graphics commands.
				if (UsedQueueFamily::findQueueFamilies(physicalDevice, *pSurface).isComplete() == KGS_FALSE)
				{
					return KGS_FALSE;
				}

				//Application can't function without support of device for swap chain extension.
				if (checkDeviceExtensionSupport(physicalDevice, deviceExtensionNames) == KGS_FALSE)
				{
					return KGS_FALSE;
				}

				//Application can't function without adequate support of device for swap chain.
				SwapChainSupportDetails swapChainSupportDetails = SwapChainSupportDetails::querySwapChainSupport(physicalDevice, *pSurface);
				if (swapChainSupportDetails.formats.empty() || swapChainSupportDetails.presentModes.empty())
				{
					return KGS_FALSE;
				}

				//Application can't function without feature of sampler anisotropy.
				if (deviceFeatures.samplerAnisotropy == VK_FALSE)
				{
					return KGS_FALSE;
				}

				return KGS_TRUE;
			}(*it);

			if (isSuitable == KGS_FALSE)
			{
				physicalDevices.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (physicalDevices.size() == 0)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}

		const std::vector<const char*>& deviceExtensions = deviceExtensionNames;
		std::sort(physicalDevices.begin(), physicalDevices.end(),
			[&](const vk::PhysicalDevice& physicalDevice1, const vk::PhysicalDevice& physicalDevice2) {
			int32_t result = 0;
			const vk::PhysicalDeviceProperties deviceProperties1 = physicalDevice1.getProperties();
			const vk::PhysicalDeviceProperties deviceProperties2 = physicalDevice2.getProperties();
			const vk::PhysicalDeviceFeatures deviceFeatures1 = physicalDevice1.getFeatures();
			const vk::PhysicalDeviceFeatures deviceFeatures2 = physicalDevice2.getFeatures();
			if (result == 0)
			{
				int32_t value1 = static_cast<int32_t>(deviceProperties1.deviceType == vk::PhysicalDeviceType::eDiscreteGpu);
				int32_t value2 = static_cast<int32_t>(deviceProperties2.deviceType == vk::PhysicalDeviceType::eDiscreteGpu);
				result = value1 - value2;
			}
			if (result == 0)
			{
				result = deviceProperties1.limits.maxImageDimension2D - deviceProperties2.limits.maxImageDimension2D;
			}
			return result > 0;
		});

		m_pPhysicalDevice = std::shared_ptr<vk::PhysicalDevice>(new vk::PhysicalDevice(*physicalDevices.cbegin()));
		LOG(plog::debug) << "Pick successfully physical device.";
	}

	void Application::_createLogicDevice(std::shared_ptr<vk::SurfaceKHR> pSurface)
	{
		UsedQueueFamily usedQueueFamily = UsedQueueFamily::findQueueFamilies(*m_pPhysicalDevice, *pSurface);
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<int32_t> uniqueFamilyIndices = { usedQueueFamily.graphicsFamily, usedQueueFamily.presentFamily };

		queueCreateInfos.resize(uniqueFamilyIndices.size());
		float queuePriority = 1.0f;

		size_t index = 0;
		for (int32_t queueFamilyIndex : uniqueFamilyIndices)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo = {
				vk::DeviceQueueCreateFlags(),                 //flags
				static_cast<uint32_t>(queueFamilyIndex),      //queueFamilyIndex
				uint32_t(1),                                  //queueCount
				&queuePriority                                //pQueuePriorities
			};
			queueCreateInfos[index] = queueCreateInfo;
			++index;
		}

		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		vk::DeviceCreateInfo createInfo = {
			vk::DeviceCreateFlags(),                         //flags
			static_cast<uint32_t>(queueCreateInfos.size()),  //queueCreateInfoCount
			queueCreateInfos.data(),                         //pQueueCreateInfos
			0,                                               //enabledLayerCount
			nullptr,                                         //ppEnabledLayerNames
			static_cast<uint32_t>(deviceExtensionNames.size()),  //enabledExtensionCount
			deviceExtensionNames.data(),                         //ppEnabledExtensionNames
			&deviceFeatures                                      //pEnabledFeatures
		};

#ifdef ENABLE_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
		createInfo.ppEnabledLayerNames = validationlayers.data();
#endif // ENABLE_VALIDATION_LAYERS

		m_pDevice = fd::createDevice(m_pPhysicalDevice, createInfo);
		m_graphicsQueue = m_pDevice->getQueue(usedQueueFamily.graphicsFamily, 0);
		m_presentQueue = m_pDevice->getQueue(usedQueueFamily.presentFamily, 0);

		LOG(plog::debug) << "Create successfully logic device.";
	}
} //namespace kgs