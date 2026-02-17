// temp

#pragma once
#include <chrono>
#include <array>
#include "VKTypes.hpp"

#include "VKSwapchain.hpp"

namespace fe {
    constexpr uint32_t maxConcurrentFrames{ 2 };

    class VKRenderer {
    public:
        VKRenderer()  = default;
        ~VKRenderer() = default;

        void Initialize(GLFWwindow* glfw_window);

    private:
        void createSurface(GLFWwindow* glfw_window);
        void createCommandPool();
        void createSwapchain();
        void createCommandBuffers();
        void createSynchronizationPrimitives();
        void setupDepthStencil();
        void setupRenderPass();
        void createPipelineCache();
        void setupFrameBuffer();

    private:
        VKSwapchain swapchain;

        // Frame counter to display fps
        uint32_t                                                    frameCounter = 0;
        uint32_t                                                    lastFPS      = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;

        // Vulkan instance, stores all per-application states
        VkInstance               instance{ VK_NULL_HANDLE };
        std::vector<std::string> supportedInstanceExtensions;

        // Physical device (GPU) that Vulkan will use
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };

        // Stores physical device properties (for e.g. checking device limits)
        VkPhysicalDeviceProperties deviceProperties{};

        // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Stores all available memory (type) properties for the physical device
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};

        /** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
        VkPhysicalDeviceFeatures enabledFeatures{};

        /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
        std::vector<const char*> enabledDeviceExtensions;

        /** @brief Set of instance extensions to be enabled for this example (must be set in the derived constructor) */
        std::vector<const char*> enabledInstanceExtensions;

        /** @brief Set of layer settings to be enabled for this example (must be set in the derived constructor) */
        std::vector<VkLayerSettingEXT> enabledLayerSettings;

        /** @brief Optional pNext structure for passing extension structures to device creation */
        void* deviceCreatepNextChain = nullptr;

        /** @brief Logical device, application's view of the physical device (GPU) */
        VkDevice device{ VK_NULL_HANDLE };

        // Handle to the device graphics queue that command buffers are submitted to
        VkQueue queue{ VK_NULL_HANDLE };

        // Depth buffer format (selected during Vulkan initialization)
        VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

        // Command buffer pool
        VkCommandPool cmdPool{ VK_NULL_HANDLE };

        // Command buffers used for rendering
        std::array<VkCommandBuffer, maxConcurrentFrames> drawCmdBuffers;

        // Global render pass for frame buffer writes
        VkRenderPass renderPass{ VK_NULL_HANDLE };

        // List of available frame buffers (same as number of swap chain images)
        std::vector<VkFramebuffer> frameBuffers;

        // Descriptor set pool
        VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };

        // List of shader modules created (stored for cleanup)
        std::vector<VkShaderModule> shaderModules;

        // Pipeline cache object
        VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

        // Wraps the swap chain to present images (framebuffers) to the windowing system
        //VulkanSwapChain swapChain;

        // Synchronization related objects and variables
        // These are used to have multiple frame buffers "in flight" to get some CPU/GPU parallelism
        uint32_t                                     currentImageIndex{ 0 };
        uint32_t                                     currentBuffer{ 0 };
        std::array<VkSemaphore, maxConcurrentFrames> presentCompleteSemaphores{};
        std::vector<VkSemaphore>                     renderCompleteSemaphores{};
        std::array<VkFence, maxConcurrentFrames>     waitFences;

        bool requiresStencil{ false };

        uint32_t apiVersion = VK_API_VERSION_1_3;
        uint32_t width      = 1920;
        uint32_t height     = 1080;

        /** @brief Default depth stencil attachment used by the default render pass */
        struct {
            VkImage        image;
            VkDeviceMemory memory;
            VkImageView    view;
        } depthStencil{};

        /** @brief Can be used by samples that use the core version of dynamic rendering */
        bool                                     useDynamicRendering{ false };
        VkPhysicalDeviceDynamicRenderingFeatures baseDynamicRenderingFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES, .dynamicRendering = VK_TRUE };
    };
} // namespace fe
