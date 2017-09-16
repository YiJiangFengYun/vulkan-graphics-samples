#ifndef GFW_APP_BASE_H
#define GFW_APP_BASE_H

#include <memory>
#include <mutex>
#include "framework/global.hpp"
#include <GLFW/glfw3.h>
#include "foundation/wrapper.hpp"
#include "framework/app/window.hpp"
#include "framework/util/thread_master.hpp"

namespace gfw
{
#ifdef DEBUG
#define ENABLE_VALIDATION_LAYERS
	const std::vector<const char*> validationlayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};
#endif // DEBUG

	const std::vector<const char*> deviceExtensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool checkDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice, std::vector<const char*> deviceExtensionNames);

	class AppBase
	{
	public:
		AppBase(uint32_t width, uint32_t height, const char *title);
		~AppBase();
		void virtual run();

		template<typename Window_T>
		void createSubWindow(uint32_t width, uint32_t height, const char *title)
		{
			std::shared_ptr<Window_T> window(new Window_T(width, height, title, m_pInstance, m_pPhysicalDevice,
				m_pDevice, m_graphicsQueue, m_presentQueue));
			/*{
				auto& subWindowsMutex = m_subWindowsMutex;
				std::lock_guard <std::mutex> lg(subWindowsMutex);
				std::vector<std::shared_ptr<Window>>& pSubWindows = m_pSubWindows;*/
			m_pSubWindows.push_back(window);

			//	//create new thread to run window.
			//	m_threadMaster.appendThread([&](std::shared_ptr<Window_T> window) {
			//		window->run();
			//		{
			//			std::lock_guard <std::mutex> lg(subWindowsMutex);
			//			pSubWindows.erase(window);
			//		}
			//	}, window);
			//}
		}

		//gettor methods
		std::shared_ptr<vk::Instance> getPVKInstance();
		std::shared_ptr<vk::PhysicalDevice> getPPhysicalDevice();
		std::shared_ptr<vk::Device> getPDevice();
		vk::Queue getGraphicsQueue();
		vk::Queue getPresentQueue();
	protected:

	private:
		AppBase(const AppBase&);

		const char* m_appName;
		uint32_t m_appVersion;
		const char* m_engineName;
		uint32_t m_engineVersion;
		std::shared_ptr<vk::Instance> m_pInstance;
		std::shared_ptr<vk::PhysicalDevice> m_pPhysicalDevice;
		std::shared_ptr<vk::Device> m_pDevice;
		vk::Queue m_graphicsQueue;
		vk::Queue m_presentQueue;
		int32_t m_width;
		int32_t m_height;
		const char *m_title;
		std::shared_ptr<gfw::Window> m_pWindow;
		std::vector<std::shared_ptr<gfw::Window>> m_pSubWindows;

		//threads
		//ThreadMaster m_threadMaster; 
		//std::mutex m_subWindowsMutex;

		void _init();
		bool _checkValidationLayerSupport();
		void _pickPhysicalDevice(std::shared_ptr<vk::SurfaceKHR> psurface);
		void _createLogicDevice(std::shared_ptr<vk::SurfaceKHR> pSurface);
		void _createWindow(std::shared_ptr<GLFWwindow> pWindow, std::shared_ptr<vk::SurfaceKHR> pSurface);

		//tool methods.
		std::shared_ptr<GLFWwindow> AppBase::_createGLFWWindow(uint32_t width, uint32_t height, const char* title);
		std::shared_ptr<vk::SurfaceKHR> AppBase::_createVKSurface(std::shared_ptr<GLFWwindow> pWindow);
		std::vector<const char*> _getRequiredExtensions();
#ifdef DEBUG
		vk::DebugReportCallbackEXT m_debugReportCallBack;
		void _setupDebugCallBack();
		void _onDebugCallBack(vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objType,
			uint64_t obj, size_t location, int32_t code, const char* layerPrefix,
			const char* msg);
		friend VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData);
#endif // DEBUG
	};
}

#endif // !GFW_APP_BASE_H