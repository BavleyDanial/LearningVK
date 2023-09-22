#include "EngineVK.h"
#include "EntryPoint.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <set>
#include <vector>

#include <glm/glm.hpp>

struct WindowProperties
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    std::string Title = "";
};

struct QueueFamilyIndices
{
    uint32_t GraphicsFamily = -1;
    uint32_t PresentFamily = -1;

    bool IsComplete()
    {
        if (GraphicsFamily < 0 && PresentFamily < 0)
            return false;
        return true;
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SandboxVK : public LearningVK::Application
{
public:
    GLFWwindow* window = nullptr;
    WindowProperties windowProps;
private:
    VkInstance vkInstance = nullptr;
    VkSurfaceKHR surface = nullptr;

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue = nullptr;
    VkDevice device = nullptr;

    VkSwapchainKHR swapChain = nullptr;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

#ifdef VK_DEBUG
    const bool vkEnableValidationLayers = true;
#else
    const bool vkEnableValidationLayers = false;
#endif

public:
    void OnInit() override
    {
        InitWindow();
        InitVulkan();
        CreateSurface();
        ChoosePhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
    }

    void OnUpdate() override
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }

        Running = false;
    }

    void OnDestruct() override
    {
        for (auto imageView : swapChainImageViews)
            vkDestroyImageView(device, imageView, nullptr);

        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(vkInstance, surface, nullptr);
        vkDestroyInstance(vkInstance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
private:
    void InitWindow()
    {
        glfwInit();

        windowProps.Width = 800;
        windowProps.Height = 600;
        windowProps.Title = "Hello Vulkan!";
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(windowProps.Width, windowProps.Height, windowProps.Title.c_str(), nullptr, nullptr);
    }

    void InitVulkan()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Vulkan!";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = uint32_t(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

#if 0
        std::cout << "Available extenstions:\n";
        for (const auto& extension : extensions)
            std::cout << extension.extensionName << "\t" << extension.specVersion << "\n";
#endif

        if (vkEnableValidationLayers && !CheckValidationLayerSupport())
        {
            std::cout << "A validation layer is not supported!" << std::endl;
            __debugbreak();
        }
        
        if (vkEnableValidationLayers)
        {
            createInfo.enabledLayerCount = uint32_t(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
        if (result != VK_SUCCESS)
            __debugbreak();
    }

    void CreateSurface()
    {
        VkResult result = glfwCreateWindowSurface(vkInstance, window, nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            std::cout << "Couldn't create surface!" << std::endl;
            __debugbreak();
        }
    }

    void ChoosePhysicalDevice()
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            std::cout << "Failed to find any GPUs that supports vulkan!" << std::endl;
            __debugbreak();
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (CheckDeviceCompatibility(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            std::cout << "Failed to find a compatible GPU!" << std::endl;
            __debugbreak();
        }
    }

    void CreateLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::array<uint32_t, 2> uniqueQueueFamilies = { indices.GraphicsFamily, indices.PresentFamily };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            if (queueFamily < 0)
                break;

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);

        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        
        createInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        
        createInfo.pEnabledFeatures = &deviceFeatures;
        
        createInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        if (vkEnableValidationLayers)
        {
            createInfo.enabledLayerCount = uint32_t(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        
        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
        if (result != VK_SUCCESS)
        {
            std::cout << "Couldn't create logical device!" << std::endl;
            __debugbreak();
        }
        
        vkGetDeviceQueue(device, indices.GraphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.PresentFamily, 0, &presentQueue);

    }

    void CreateSwapChain()
    {
        SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(physicalDevice);

        VkExtent2D extent = ChooseSwapExtent(swapChainDetails.surfaceCapabilities);
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainDetails.surfaceFormats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainDetails.presentModes);

        uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;
        if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && imageCount > swapChainDetails.surfaceCapabilities.maxImageCount)
            imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily, indices.PresentFamily };

        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
        if (result != VK_SUCCESS)
        {
            std::cout << "Couldn't create swapchain!" << std::endl;
            __debugbreak();
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void CreateImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (int i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
            if (result != VK_SUCCESS)
            {
                std::cout << "Couldn't create image view number " << i << "!" << std::endl;
                __debugbreak();
            }
        }

        
    }

    bool CheckDeviceCompatibility(const VkPhysicalDevice& device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);
        bool extensionSupport = CheckDeviceExtensionSupport(device);

        bool swapChainSupport;
        if (extensionSupport)
        {
            SwapChainSupportDetails details = QuerySwapChainSupport(device);
            swapChainSupport = !details.surfaceFormats.empty() && !details.presentModes.empty();
        }

        return indices.IsComplete() && extensionSupport && swapChainSupport;
    }

    bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device)
    {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            VkBool32 presentSupport;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.GraphicsFamily = i;
            if (presentSupport)
                indices.PresentFamily = i;
        }

        return indices;
    }

    SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        details.surfaceFormats.resize(formatCount);    
        if (formatCount > 0)
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        details.presentModes.resize(presentModeCount);
        if (presentModeCount > 0)
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());



        return details;
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& format : availableFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
                return format;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return presentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width == UINT32_MAX)
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                uint32_t(width),
                uint32_t(height)
            };

            actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            
            return actualExtent;
        }

        return capabilities.currentExtent;
    }


    std::vector<const char*> GetRequiredExtensions()
    {
        uint32_t glfwExtensionCount;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (vkEnableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    bool CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layer : availableLayers)
            {
                if (strcmp(layerName, layer.layerName))
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }
};

LearningVK::Application* LearningVK::CreateApplication()
{
	return new SandboxVK();
}