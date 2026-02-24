# Forr-Engine Devlog

## 25.02.2026
### Goal
    Work with Swaphchain
    Create Pipeline
    Create ...

    Draw a triangle
    Create Vulkan GPUResourceManager like in RendererOpenGL

    Example of VertexBuffer :
    ```cpp
    struct VertexBuffer {
        fe::vk::Buffer buffer;
        fe::vk::DeviceMemory memory;
        VkDeviceSize size{};
        void* mapped{};
    };
    ```

### Done
    -

### Problem
    -

---

## 24.02.2026
### Goal
    Create Swaphchain
    Create Pipeline
    Create ...

    Draw a triangle
    Create Vulkan GPUResourceManager like in RendererOpenGL

    Example of VertexBuffer :
    ```cpp
    struct VertexBuffer {
        fe::vk::Buffer buffer;
        fe::vk::DeviceMemory memory;
        VkDeviceSize size{};
        void* mapped{};
    };
    ```

### Done
    VulkanSwapchain created
    VulkanSwapchain lifecycle done, all that's left is logic
    CommandBuffers in progress ( added )

### Problem
    -

---

## 23.02.2026
### Goal
    Create VulkanSwapchain
    Create VulkanDevice

    Draw a triangle
    Create Vulkan GPUResourceManager like in RendererOpenGL

    13:04 - next step :
    - get queues
    - create VulkanSwapchain
    - create surface
    - ...

### Done
    argv -> args in vk::ApplicationDesc
    VulkanContext updated
    Added : 
    - VKGetQueueFamilyProperties()
    - VKGetSupportedExtensions()
    - VKSetupQueueFamilies();
    - VKCreateDevice() ( in progress )
    - VKCreateCommandPool() ( in progress )
    VKTools.hpp added
    VKGetQueueFamilyProperties() -> VKSetupQueueFamilyProperties()
    VKGetSupportedExtensions() -> VKSetupSupportedExtensions()
    VKSetupQueueFamilies() -> VKGetQueueFamilyInfos()
    VKCreateDevice() done
    VKCreateCommandPool() done
    getQueueFamilyIndex() now static
    VKCreateInstance2() created
    VKCreateInstance() replaced
    little fixes provided
    bug fixed
    VKSetupQueues() done
    VulkanSwapchain created ( but is it really needed ? )
    VKCreateSurface() done
    VKSetupSurfaceColorFormat() done
    VKSetupQueueNodeIndex() done
    VKCreateSwapchain() created
    GLFW window monitors resolved ( added new logic )
    Fullscreen mode added
    VSync added

### Problem
    -

---


## 22.02.2026
### Goal
    Think about Vulkan architecture
    Devide Vulkan initialization by functions

    Read article : https://habr.com/ru/articles/992894/

    Create VulkanDevice ( progress )
    Create VulkanSwapchain

### Done
    Deviding Vulkan code in progress
    fe::vk::create_and_wrap() removed
    mini debug messanger added
    GLFW a little bit limited
    Application name added
    m_Desc -> m_Description ( I always prefer full names )
    Worked with Vulkan architecture ( still in progress )
    fe::vk::DeviceMemory added
    VulkanContext updated
    VulkanContext updated again
    MeshID updated
    VulkanContext.hpp added

### Problem
    The article uses too modern C++ and VkHpp
    While copying code from Sascha Willems Vulkan Examples ( MIT License ) ( www.saschawillems.de )
        I realized, that there are too many Vulkan Tools.
        For example : vks::initializers::* or like in general, all vks::*.
        Some vks::Buffer but I have different system. My fe::vk::Buffer is like ::vk::UniqueBuffer.
        So, I guess for now I will just try to consolidate the architecture

---

## 21.02.2026
### Goal
    Consolidate the 2 month plan
    Think about Vulkan architecture

### Done
    2 month plan consolidated

### Problem
    -

---

## 20.02.2026
### Goal
    Consolidate the 2 month plan
    Think about Vulkan architecture

### Done
    -

### Problem
    Skipped data

---

## 19.02.2026
### Goal
    Consolidate the 2 month plan
    Think about Vulkan architecture

### Done
    -

### Problem
    I am very tired. I will do it tomorrow

---

## 18.02.2026
### Goal
    Consolidate the 2 month plan
    Think about Vulkan architecture

    VulkanDevice - is a class, where I put physical and logical devices
        and some helper functions. I'll try to create it.

### Done
    VulkanDevice started. Initialize() added

### Problem
    I've got MS Visual Studio 2026 in my .sln file.
    I specifically created the project on old 2022th version,
        because 2026th version is shit right now.
    Clang-Tidy from Clang Power Tools does not work.
    GUI is inconvenience, Microsoft didn't fix it.
    I wrote them about inconvenience GUI using the newest version of 2026th VS,
        so they told me that this is already fixed and gave me link to other's message.
    So, if I write you about the problem on the newest version, maybe that means that you
        didn't fix the problem or fixed it badly?
    Anyway, I didn't tell about it. I will better wait till this 2026th version will become useful.
    It's like CS2. CS:GO was much better, CS:GO was that game I so much liked. But people say,
        that CS:GO on release is also was hated like CS2 on release. And I trust it. I wish the time,
        when CS2 will become that very game.

    But Microsoft is not Valve and Visual Studio Installer is not Steam, 
        also like MS Visual Studio is not Counter-Strike.
    So, I can use whatever version I want. And I will use MS Visual Studio 2022.
    But today I'll try MS Visual Studio 2026, maybe I'll like it.

---

## 17.02.2026
### Goal
    Consolidate the 2 month plan
    Consolidate Vulkan architecture
    Use Sascha Willems Vulkan Examples to build the architecture

    Just separate Vulkan initialization by different methods for now
    
    Add temp classes
    - VKRenderer
    - VKSwapchain
    - VKBuffer
    - VKDevice

    Think about Vulkan architecture

### Done
    VKRenderer added
    VKSwapchain added
    VKBuffer added
    VKDevice added
    VKInitializers added

### Problem
    I don't know how to make Vulkan architecture, I am tired

---

## 16.02.2026
### Goal
    Consolidate the 2 month plan
    Consolidate Vulkan architecture
    
    
### Done
    -

### Problem
    -

---

## 15.02.2026
### Goal
    Consolidate the 2 month plan
    Create Vulkan architecture

    Handles needs for :
    - Instance
    - Device
    - SurfaceKHR
    - SwapchainKHR
    - Buffer
    - Image
    - ImageView
    - Sampler
    - ShaderModule
    - RenderPass
    - Framebuffer
    - Pipeline
    - PipelineLayout
    - DescriptorSetLayout
    - DescriptorPool
    - CommandPool
    - Fence
    - Semaphore
    - Event
    
### Done
    Trying to add Vulkan types

    Handles done for :
    - all

    fe::vk::create_and_wrap() added

### Problem
    -

---

## 14.02.2026
### Goal
    Consolidate the 2 month plan
    Create Vulkan triangle
### Done
    Vulkan window successfully created
    Vulkan triangle done
### Problem
    -

---

## 13.02.2026
### Goal
    Consolidate the 2 month plan
    Create Renderer aka Graphics
### Done
    Initialize()s removed
    Renderer creating
    OpenGL context created
    Glad added ( nothing in External, yeah, idk,
        README says that I have to just download it by zip, so I did it )
    GLM added
    Triangle added, but no Shader yet
    PathManager added
    Shader added ( using legacy code )
    Triagnle done
    Vulkan added
### Problem
    -

---

## 12.02.2026
### Goal
    Create platform system
    Create GLFW window
    Consolidate the 2 month plan
### Done
    Platform system created
    Colorful logging returned
    GLFW window created
### Problem
    -

---

## 11.02.2026
### Goal
    Create ForrEditor
    Create ForrGame
    Add Core
    Create window
    Create Application Layers
    Create long-time plan. At least for 2 months
### Done
    Switched progress.md; Now the older days are at the bottom
        and the newer ones are at the top
    ILayer added
    Core added
    attributes.hpp updated
    IPlatformSystem created
    IWindow created
    ForrEditor created
    ForrGame created
    Application layers created
    Working on 2 month plan
### Problem
    -

---

## 10.02.2026
### Goal
    Create first project and class : ForrPlayer and Application
### Done
    Project and class are created.  
### Problem
    -

---

## 07.02.2026
### Goal
    Delete all code. Recreate the project
### Done
    -
### Problem
    -

---

## 06.02.2026
### Goal
    Add modular namespaces
    Create Forr.Shared
    Add Forr.Shared to README.md
    Unload or remove unused projects for now
    Remove services
    Create Window class, which will ask Forr.Platform to create GLFW window 
### Done
    Modular namespaces added
    Forr.Shared added
    Unused projects removed
    README.md updated
    Services removed
    Window created
### Problem
    -

---

## 05.02.2026
### Goal
    Add modular namespaces
### Done
    -
### Problem
    Got ill. Fell asleep

---

## 04.02.2026
### Goal
    Create GLFW window in Forr.Platform
    Configure relationship between Forr.Platform, Forr.Engine and Forr.Editor
### Done
    GLFW added
### Problem
    -

---

## 03.02.2026
### Goal
    Configure Forr.Core
    Add attributes.hpp and custom_allocators.hpp etc
    Setup file handler
### Done
    Forr.Core configured
    attributes.hpp, custom_allocators.hpp, logging.hpp/cpp and pointer.hpp are added
    Handler recreated
    + colorful logging
    + DLL linking
    + Forr.Core refers configured
### Problem
    -

---