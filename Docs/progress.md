# Forr-Engine Devlog

## 23.07.2026
### Goal
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )

### Done
    THIRD SEASON OF MUSHOKU TENSEI JUST RELEASED AT 04.07.2026!!!!!
        I don't know what I should watch first : Rick and Morty or Mushoku Tensei~
        And do I have to watch whole anime from the first part as a wanted to do this summer ?
    render_graph::CreateCommandList provided

### Problem
    JudeLow's Minecraft trap videos in YouTube

## 21.07.2026
### Goal
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )

### Done
    culling done

### Problem
    -

## 19.07.2026
### Goal
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )

### Done
    RenderGraph in progress
    Kahn's algorithm implemented
    RenderGraph::Compile() almost done

### Problem
    -

## 18.07.2026
### Goal
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )

### Done
    thinking about RenderGraph...
    RenderGraph integrated to Application

### Problem
    -

## 17.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    thinking about RenderGraph...
    hashed_string and fixed_hashed_string added
    render graph skeleton logic done

### Problem
    -

## 15.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    ShaderImporter done
    bug fixed : 'resource::resource_t' replaced 'typename' in template functions of resource management
    SlangParser done
    string.hpp added

### Problem
    -

## 14.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    understood how to get structures from Slang file via Slang Reflection API
    reflection in progress
    ShaderReflectedData changed

### Problem
    borken PCH : I have to wait 21 seconds every compilation

## 13.07.2026
### Goal
    create separate class to handle Slang
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    Trying to save Slang serialized data
    SlangParser added

### Problem
    Very hard to start and continue the work

## 12.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    ReflectedParameter --> ReflectedDescriptor

### Problem
    -

## 11.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    thinking about RenderGraph
    fe::resource::ShaderProgram reflected info moved to fe::shader::* in GPUTypes.hpp

### Problem
    -

## 09.07.2026
### Goal
    create render graph logic ( render passes )
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    thinking about RenderGraph

### Problem
    -

## 07.07.2026
### Goal
    allocate material buffer in 'fe::ResourceManager', using 'fe::Arena';
        allocate big SSBO ( AZDO ) in renderer for materials;
        see something on the screen
    make a video for YouTube, when see something on the screen
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    create render graph logic ( render passes )
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

    ( BDA for Vulkan | ByteAddressBuffer for OpenGL )
    ( use fe::Arena to store fe::resource::Material's buffers' data )

### Done
    shader buffers creation provided to OpenGL

### Problem
    -

## 02.07.2026
### Goal
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

### Done
    Slang reflection decompilations done
    Slang reflection in progress ( last settings : 'descriptor_type' for Resources, 'is_bindless' and 'array_size' )
    'descriptor_type' for Resources, 'is_bindless' and 'array_size' are fixed
    Slang reflection is done
    push constants' validation provided
    validation adding in progress

### Problem
    -

## 01.07.2026
### Goal
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

### Done
    Slang reflection in progress

### Problem
    -

## 30.06.2026
### Goal
    review materials system
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations )

### Done
    fe::resource::ShaderProgram reviewed
    __cplusplus fixed

### Problem
    -

## 29.06.2026
### Goal
    review materials system
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    working on Slang reflection

### Problem
    -

## 25.06.2026
### Goal
    create buffers ( OpenGL/Vulkan ) via reflected information from the shader
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    -

### Problem
    completely disappointed in Slang.
        Maybe better write code in HLSL ( which I almost learned, learning Slang ) and use SPIR-V and SPIRV-Cross ?
    Vulkan backend still does not work ( maybe a mounth ) because of runtime error of VMA
    OpenGL backend does not work because of lack of buffers. Vulkan needs buffers too

## 24.06.2026
### Goal
    validate Slang reflection
    fix Vukan VMA error
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    Slang reflection is done
    Slang reflection validation provided

### Problem
    -

## 22.06.2026
### Goal
    rewrite Slang reflection
    fix Vukan VMA error
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    starting to understand Slang Reflection API

### Problem
    Slang Reflection API is got a bug

## 21.06.2026
### Goal
    do Slang reflection
    fix Vukan VMA error
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    Slang reflection fixed <-- ( 22.06.2026 ) who wrote this ?

### Problem
    -

## 20.06.2026
### Goal
    do Slang reflection
    fix Vukan VMA error
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    NOTE : your own code after Unreal Engine 4/5 feels much better
    trying to implement reflection

### Problem
    Some other project ( done today )

## 14.06.2026
### Goal
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    FAAAAHH sound works in VS2026 even without a specific extension.
        Maybe this IDE is not that bad I was thinking. But it's GUI is still awkward
    Provided FAAAHH sound for Windows critical error window and when build fails
    returned my code from the stash
    fe::resource::ShaderProgram::SourceCode is unified now

### Problem
    -

## 13.06.2026
### Goal
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    Slang shader compiling almost resolved

### Problem
    my IDE just broken, so, I have to switch my favourite VS2022 to this shity modern VS2026,
        because its compiler, v145, works, unlike v143, which just cracked after working with UE5

## 12.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    remove fe::resource::Shader. Create fe::resource::ShaderProgram instead
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    fe::VulkanResourceManager::createMesh() finally turned from 165 lines of code to only 6
    RVO bug fixed ( all 'return std::move(...)' removed )
    fe::createImage() added
    Vulkan mipmaps generating done. VulkanTexture creation finally added
    Slang added
    Slang integration in progress. ShaderCompiler and ShaderReflector removed
    fe::resource::Shader --> fe::resource::ShaderProgram. shader errors resolved

### Problem
    Slang shaders compilation

## 11.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VKTools.hpp : createBuffer(), runOneTimeCommands() and createDeviceLocalBuffer() are added

### Problem
    -

## 09.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate code
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    -

### Problem
    Russian language exam ( it is harder than C++ templates. I'm not even Russian! )

## 08.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate code
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VulkanRAII.hpp : template hell added

### Problem
    Russian language exam

## 07.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate code
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VMA submodule added
    VMA added
    fe::vk::Allocator added
    fe::RendererVulkan::m_Allocator added. VulkanRAII.hpp reviewed

### Problem
    -

## 04.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate code
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    remembered STBI using logic. It can be used for VMA as well

### Problem
    -

## 03.06.2026
### Goal
    provide tools to decrease Vulkan boilerplate code
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    provide debug tools with Dear ImGui
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VMA added

### Problem
    Vulkan boilerplate

## 02.06.2026
### Goal
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    Trying to create fe::VulkanTexture by fe::resource::Texture

### Problem
    Tired. I'll leave this for tomorrow

## 31.05.2026
### Goal
    provide texture's concept to Vulkan ( don't push to the shader, CPU only )
    chnage GLSL to Slang
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VulkanResourceManager reviewed like OpenGLResourceManager before
    VulkanTexture is not empty now

### Problem
    -

## 30.05.2026
### Goal
    provide textures
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    fe::resource::Material::Sampler added
    here done a lot of things that wasn't written

### Problem
    GL_ARB_bindless_texture is not allowed. Needs to be switched from GLSL to Slang

## 29.05.2026
### Goal
    provide textures
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( this all needs code review )

### Done
    fe::OpenGLMaterial removed ( unused abstraction layer ). Now fe::resource::Material --> fe::OpenGLShaderProgram/fe::VulkanMaterial
    textures almost done

### Problem
    -

## 28.05.2026
### Goal
    provide textures
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( this all needs code review )

### Done
    almost understood how it should work

### Problem
    -

## 26.05.2026
### Goal
    provide textures
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( this all needs code review )

### Done
    texture coords pushing provided for OpenGL
    texture coords pushing provided for Vulkan

### Problem
    -

## 24.05.2026
### Goal
    provide the same of OpenGL
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( this all needs code review )

### Done
    OpenGLShaderProgram changed
    lighting provided for OpenGL
    Shader.hpp/Shader.cpp removed

### Problem
    -

## 23.05.2026
### Goal
    provide vertex normals pushing to the shader ( it's in gLTF )
    provide shader with lighting
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )
    ( this all needs code review )

### Done
    normals provided. light provided

### Problem
    -

## 22.05.2026
### Goal
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    Suzanne added. Trying to fix window resizing

### Problem
    -

## 21.05.2026
### Goal
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    VulkanUniformBuffer, VulkanStorageBuffer --> VulkanShaderBuffer
    light data pushing provided

### Problem
    -

## 20.05.2026
### Goal
    provide RenderPacket
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    RenderPacket provided

### Problem
    -

## 19.05.2026
### Goal
    provide bindless rendering
    devide scene's and material's descriptor sets
    provide RenderPacket
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix window resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    features for bindless rendering has adding provided
    bindless descriptor set layout initialization configured
    shader reflection's got better
    devided scene's and material's descriptor sets

### Problem
    -

## 18.05.2026
### Goal
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

    provide RenderPacket :
    ```hpp
    struct RenderPacket {
        CameraData camera;
        std::vector<GPULight> visible_lights;
        std::vector<RenderCommand> draw_calls; 
    };
    
    class IRenderer {
    public:
        virtual void SubmitFrame(const RenderPacket& packet) = 0;
    };
    ```

### Done
    ( check stash )

### Problem
    -

## 17.05.2026
### Goal
    provide normals pushing to shader ( it's in gLTF )
    provide shader with lighting
    ( fix resizing for GL/VK )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

    provide RenderPacket :
    ```hpp
    struct RenderPacket {
        CameraData camera;
        std::vector<GPULight> visible_lights;
        std::vector<RenderCommand> draw_calls; 
    };
    
    class IRenderer {
    public:
        virtual void SubmitFrame(const RenderPacket& packet) = 0;
    };
    ```

### Done
    -

### Problem
    -

## 16.05.2026
### Goal
    provide sorting for OpenGL
    provide FrameData for Vulkan ( already done, just move it into a struct )
    create different materials

    ( rename 'model_index' in shader to 'instance_index' )
    ( do not create GPU resources if they're already created --> InitializeGPUResources() )

### Done
    sorting for OpenGL provided
    FrameData structure for Vulkan provided
    ResourceLookupTable removed ( unused idk )
    renamed 'model_index' in shader to 'instance_index'
    material overriding provided
    different materials created
    GL/VK choosing provided. Bugs fixed

### Problem
    -

## 16.05.2026
### Goal
    provide sorting for Vulkan/OpenGL
    provide FrameData ( already done, just move it into a struct )
    create different materials

### Done
    sorting for Vulkan provided

### Problem
    -

## 15.05.2026
### Goal
    provide sorting for Vulkan/OpenGL
    provide FrameData
    create different materials

### Done
    RenderSystem progressed
    adding ECS
    rendering done

### Problem
    -

## 14.05.2026
### Goal
    decide what I have to use : MeshProxy? How to create sorting
    provide sorting for Vulkan/OpenGL
    provide FrameData
    create different materials

### Done
    create RenderMeshEntry. Use only fe::pointer<fe::resource::Model> in fe::MeshComponent

### Problem
    -

## 13.05.2026
### Goal
    provide std::vector<DrawCommand> m_RenderQueue{} for RendererVulkan
    provide sorting for Vulkan/OpenGL
    provide FrameData
    create different materials

### Done
    m_RenderQueue provided. RendererSystem added. EnTT added

### Problem
    -

## 12.05.2026
### Goal
    provide materials for Vulkan
    provide FrameData

### Done
    ForrAI pet-project
    planning
    ( pipeline cache removed - keep in mind )
    VulkanMaterial added
    VulkanMaterial creation provided
    pipeline creation moved to materials - everything works correctly

### Problem
    -

## 03.05.2026
### Goal
    provide new GPU resource management for Vulkan
    provide materials for Vulkan

### Done
    new GPU resource manager provided for Vulkan

### Problem
    -

## 30.04.2026
### Goal
    provide new GPU resource management for Vulkan
    devide UBO/SSBO and push_constants in fe::resource::Material

### Done
    OpenGLResourceManager finalized
    OpenGL renderer fixed

### Problem
    -

## 26.04.2026
### Goal
    devide UBO/SSBO and push_constants in fe::resource::Material
    provide push_constants/uniforms for pushing indices to draw the models
    provide the same for Vulkan backend
    
    ( maybe you'll need to provide caching shader properties ( vertex and fragment ) in material )
    ( create a helper for gpu/cpu resource - it's unreal to manage )
    
    remove GPUResourceLookupTable and put some GPUHandle to the resource

### Done
    GPUResourceLookupTable has removed
    GPUHandle<> added
    IMPORT_INSTANCE -> IMPORTER_INSTANCE
    GPU resource management of OpenGL is rewrote

### Problem
    -

## 22.04.2026
### Goal
    devide UBO/SSBO and push_constants in fe::resource::Material
    provide push_constants/uniforms for pushing indices to draw the models
    provide the same for Vulkan backend
    
    ( maybe you'll need to provide caching shader properties ( vertex and fragment ) in material )
    ( create a helper for gpu/cpu resource - it's unreal to manage )
    
    remove GPUResourceLookupTable and put some GPUHandle to the resource

### Done
    -

### Problem
    -

## 19.04.2026
### Goal
    move UBO creating from Renderer to material or shader creating
    remove ShaderData from Renderer
    devide UBO/SSBO and push_constants in fe::resource::Material
    provide push_constants/uniforms for pushing indices to draw the models
    provide the same for Vulkan backend
    
    ( maybe you'll need to provide caching shader properties ( vertex and fragment ) in material )
    ( create a helper for gpu/cpu resource - it's unreal to manage )
    
    remove GPUResourceLookupTable and create ...

### Done
    removed UBO creating. using SSBO now
    scene data moved to renderer ( in right way )

### Problem
    -

## 18.04.2026
### Goal
    provide push_constants/uniforms for pushing indices to draw the models
    
### Done
    Saint-Petersburg-Tour-2026 : ~105GB of content

### Problem
    -

## 14.04.2026
### Goal
    provide OpenGL backend resources destroying ( create fe::gl::* )
    translate material from glm::vec3 color to std::vector<uint8_t> buffer
    provide push_constants/uniforms for pushing indices to draw the models
    
### Done
    RAII provided for OpenGL backend
    Material now using std::vector<uint8_t> buffer instead of glm::vec3

### Problem
    -

## 13.04.2026
### Goal
    provide OpenGL backend resources destroying ( create fe::gl::* )
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    provide textures to the material
    
### Done
    OpenGLShaderProgram now contains fe::gl::ShaderProgram as a proxy structure
    OpenGL Shader Program destroyed correctly
    RAII provided for OpenGL backend

### Problem
    Feel bad. Vomit

## 12.04.2026
### Goal
    provide OpenGL backend resources destroying ( create fe::gl::* )
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    provide textures to the material
    
### Done
    OpenGLRAII.hpp added

### Problem
    -

## 11.04.2026
### Goal
    add Engine/User folders
    create new system for PathManager
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    provide textures to the material
    
### Done
     Engine/User and Shared folders created

### Problem
    -

## 10.04.2026
### Goal
    create ShaderCompiler {
        write default shaders ( GLSL )
        add this : https://github.com/google/shaderc
        compile that shaders via the shaderc ( don't forget about defines )
        create something like 'ShaderReflector' to use it in ShaderImporter and ShaderCompiler
        ...
        provide GraphicsBackend to ResourceCreator
    }
    add Engine/User folders
    ( see something on the screen )
    provide 'official' creation of default materials
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    using reflected info from the shader, build parameters in the material and insert std::vector<char> or something
    provide textures to the material
    
### Done
     shaderc linking problem resolved
     ShaderReflector added
     ShaderImporter now uses ShaderReflector
     ResourceManagementContext added
     ShaderImporter now uses ShaderCompiler
     shader loading fixed
     seen something on the screen

### Problem
    -

## 09.04.2026
### Goal
    create ShaderCompiler {
        write default shaders ( GLSL )
        add this : https://github.com/google/shaderc
        compile that shaders via the shaderc ( don't forget about defines )
        create something like 'ShaderReflector' to use it in ShaderImporter and ShaderCompiler
        ...
    }

    ( see something on the screen )
    provide 'official' creation of default materials
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    using reflected info from the shader, build parameters in the material and insert std::vector<char> or something
    provide textures to the material
    
### Done
     ShaderCompiler added

### Problem
    Idk how to include shaderc. I'm using shaderc from VulkanSDK for now

## 08.04.2026
### Goal
    create ShaderCompiler
    ( see something on the screen )
    provide 'official' creation of default materials
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    using reflected info from the shader, build parameters in the material and insert std::vector<char> or something
    provide textures to the material
    
### Done
     -

### Problem
    Got ill

## 07.04.2026
### Goal
    create ShaderCompiler
    ( see something on the screen )
    provide 'official' creation of default materials
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    using reflected info from the shader, build parameters in the material and insert std::vector<char> or something
    provide textures to the material
    
### Done
     -

### Problem
    Got ill

## 04.04.2026
### Goal
    night of 03.04.2026 :
    Create fe::resource::Material structure with only glm::vec3 color
    Provide material importing
    Provide material creation in backend
    Provide material using ( push its parameter to the shader )
    ( shader are loaded in the graphics backend like before. Only OpenGL for now )
    day of 04.04.2026 :
    move shaders into material ( loading logic )
    provide 'official' creation of default materials
    provide OpenGL backend resources destroying
    ( think about how and where create shader's UBOs )
    ( to use model id you can check the shader for name like 'model_id' while reflecting )
    ( maybe make warnings if Vulkan backend gets a shader with single uniforms or if OpenGL one gets a shader with push_constants )
    using reflected info from the shader, build parameters in the material and insert std::vector<char> or something
    provide textures to the material
    
### Done
    fe::IRenderer::createDefaultMaterials() removed
    fe::resource::Material with glm::vec3 color added
    material 'importing' provided ( I can't make the real importing for now )
    material creation in backend provided
    material using provided ( the code is really messed up )
     
### Problem
    Got ill

## 02.04.2026
### Goal
    Redesign resource management : extension dependent --> type dependent
    Create default material for gLTF
    remove IShader
    Create new Shader class for OpenGL
    Add UBO ( and SSBO ) for Camera and other scene data

### Done
    fe::ResourceManager now uses full paths instead of relative
    Resource Management logic got better. Material logic now can be made. Some bugs fixed
    fe::pointer::m_generation now initializes as std::numeric_limits<handle_t>::max() like fe::pointer::m_index. added operator! for fe::pointer<>
    default gltf material created
    IShader removed
    fe::pointer operator! -> fe::pointer operator bool

### Problem
    Got ill

## 01.04.2026
### Goal
    Create fe::resource::Shader containing the source
    Provide shader reflection via SPRV-Reflect and fill up fe::resource::Material with it
    Provide fe::resource::Material saving with ResourceCreator
    Provide unified material importing aka MaterialImporter
    Add UBO ( and SSBO ) for Camera and other scene data

### Done
    -

### Problem
    Got ill

## 31.03.2026
### Goal
    Create fe::resource::Shader containing the source
    Provide shader reflection via SPRV-Reflect and fill up fe::resource::Material with it
    Provide fe::resource::Material saving with ResourceCreator
    Provide unified material importing aka MaterialImporter
    Add UBO ( and SSBO ) for Camera and other scene data

### Done
    Importers now return a value
    Shader update
    ShaderImporter added
    ShaderImporter done
    Files\\Shaders -> Files\\Resources\\Shaders
    Shaders properties are loading

### Problem
    Sleep debt

## 30.03.2026
### Goal
    Create fe::resource::Shader containing the source
    Provide shader reflection via SPRV-Reflect and fill up fe::resource::Material with it
    Provide fe::resource::Material saving with ResourceCreator
    Provide unified material importing aka MaterialImporter
    Add UBO ( and SSBO ) for Camera and other scene data

### Done
    -

### Problem
    -

## 29.03.2026
### Goal
    Provide shader reflection via SPRV-Reflect and fill up fe::resource::Material with it
    Provide fe::resource::Material saving with ResourceCreator
    Provide unified material importing aka MaterialImporter
    Add UBO ( and SSBO ) for Camera and other scene data

### Done
    I finally got my Sony Camera ZV-E10 - I'm gonna be a bloger! or vloger, idk

### Problem
    Time management

## 28.03.2026
### Goal
    Provide unified material importing aka MaterialImporter
    Create ResourceCreator and material creation in it
    Create GUID for resources
    Remove ShaderImporter
    Create shader reflection and unified material for it
    Add UBO ( and SSBO ) for Camera and other scene data

    ```hpp
    struct FORR_API Material {
    public:
        struct FORR_API Property {
        public:
            enum class Type {
                FLOAT,
                INT
                // ...
            };

            uint32_t offset{};
            uint32_t size{};
            uint32_t count{};
            Type     type{};

            Property()  = default;
            ~Property() = default;
        };

        std::unordered_map<std::string, Property> properties{};
        std::vector<uint8_t>                      buffer{};


        Material()  = default;
        ~Material() = default;

        FORR_RESOURCE_BODY(Material)
    };
    ```

### Done
    came up with extension name : .forr_<type>. ".forr_meta", ".forr_material"
    MaterialImporter added
    ResourceCreator added
    fe::PathManager::getMetadataExtension() and fe::PathManager::getMaterialExtension() are added
    trying to add fe::ResourceCreator::createMaterial()
    GUID added
    SPIRV-Reflect added
    ShaderImporter removed

### Problem
    I don't want to create any serialization till C++26

## 27.03.2026
### Goal
    Create fe::resource::Shader and maybe use PIMPL in it
    Create shader reflection and unified material for it
    Provide unified material importing aka MaterialImporter
    Add UBO ( or SSBO ) for Camera and other scene data

    ```cpp
    namespace fe::resource {
        struct Material {
        public:
            enum class PropertyType {
                // ...
            };

            struct Property {
            public:
                size_t       offset{};
                size_t       size{};
                PropertyType type{};

                Property()  = default;
                ~Property() = default;
            };

            void set_float(std::string name, float value) { // logic ( this mustn't be here. it's POD )
                auto& prop = properties[name];
                memcpy(buffer.data() + prop.offset, &value, sizeof(float));
            }

            std::unordered_map<std::string, Property> properties{};
            std::vector<uint8_t>                      buffer{};
            IShader*                                  linked_shader_ptr{};

            Material()  = default;
            ~Material() = default;

            FORR_RESOURCE_BODY(Material)
        };
    } // namespace fe::resource
    ```

### Done
    the first day being 16
    fe::resource::Material structure changed
    fe::resource::Shader added
    ShaderImporter added
    trying to provide Shader for OpenGL

### Problem
    I don't know how to organize shaders

## 26.03.2026
### Goal
    Create shader reflection and unified material for it
    Add UBO ( or SSBO ) for Camera and other scene data

    ```cpp
    namespace fe::resource {
        struct Material {
        public:
            enum class PropertyType {
                // ...
            };

            struct Property {
            public:
                size_t       offset{};
                size_t       size{};
                PropertyType type{};

                Property()  = default;
                ~Property() = default;
            };

            void set_float(std::string name, float value) { // logic
                auto& prop = properties[name];
                memcpy(buffer.data() + prop.offset, &value, sizeof(float));
            }

            std::unordered_map<std::string, Property> properties;
            std::vector<uint8_t>                      buffer;
            fe::pointer<IShader>                      linked_shader{};

            Material()  = default;
            ~Material() = default;

            FORR_RESOURCE_BODY(Material)
        };
    } // namespace fe::resource
    ```

### Done
    Happy Birthday to me !!

### Problem
    -

## 23.03.2026
### Goal
    -

### Done
    "Not now, but" moved to not-now-but.md

### Problem
    -

## 25.03.2026
### Goal
    Add fe::GraphicsBackend to fe::ShaderDesc
    Create models matrices storing and passing to the shader.
        Push models matrices in fe::IRenderer::EndFrame() and push model id while drawing ( push_constants or glUniform1i )

### Done
    fe::GraphicsBackend added to fe::ShaderDesc
    Model matrices pushing provided to RendererOpenGL. Without abstractions yet

### Problem
    I don't understand how I supposed to use IShader.
    I don't think that I will directly call it from the application.
    In the application I most likely will use fe::Material or something like that

## 23.03.2026
### Goal
    -

### Done
    "Not now, but" moved to not-now-but.md

### Problem
    -

## 22.03.2026
### Goal
    Create minimal IShader class and create different shaders for Vulkan and OpenGL

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui
    - create own extension to cache model parts and unload the model from RAM to disc and load it again when needs
    - maybe move this "Not now, but" to other file ?

### Done
    IShader added
    ShaderOpenGL added

### Problem
    -

## 21.03.2026
### Goal
    Rewrite OpenGL renderer architecture
    Add push_constant
    Add Dear ImGui

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    OpenGLResourceManagement folder created
    ResourceLookupTable -> GPUResourceLookupTable
    OpenGL drawing with new architecture done
    OpenGL VSync bug fixed
    push_constants added for Vulkan

### Problem
    -

## 20.03.2026
### Goal
    Rewrite VulkanPrimitive ( OpenGLPrimitive )
    Add push_constant
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    Trying to review
    gTLF, unified logic and Vulkan types are reviewed
    ResourceLookupTable created. VulkanResourceCreator created
    VulkanResourceStorage added
    VulkanResourceManagement folder added
    class VulkanResourceStorage -> struct VulkanResourceStorage. VulkanResourceManager::CreateResource() -> VulkanResourceManager::Create*()
    ResourceLookupTable done. But idk is it works or no cuz freaking VS 2022 don't wonna work on freaking i9-14900K with 64GB RAM
    VulkanResourceImporter -> VulkanResourceCreator
    new architecture almost provided for Vulkan
    new architecture's declaration is almost done
    VulkanResourceManager::CreateResource() unified
    gpu resource manager is done for Vulkan. Texture.hpp ( OpenGL ) removed
    the model is drawn with new architecture on Vulkan

### Problem
    -

## 19.03.2026
### Goal
    Rewrite VulkanPrimitive ( OpenGLPrimitive )
    Add push_constant
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    -

### Problem
    YouTube, Instagram, sleeping

## 18.03.2026
### Goal
    Rewrite VulkanPrimitive ( OpenGLPrimitive )
    Add push_constant
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    Put some TODOs

### Problem
    -

## 17.03.2026
### Goal
    Add push_constant
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    -

### Problem
    Lack of sleep

## 15.03.2026
### Goal
    ...
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - create Shader class for both backends. When the project will be able to be "compiled", 
        that Shader class should compile all shader files. Now I'm tired to compile shaders manually.
        Maybe create IShader that will compile shader files and ShaderVulkan/ShaderOpenGL, 
        that will create needed specific objects for backends
    - add textures
    - add Dear ImGui

### Done
    Clear color ability synchronized for all backends
    Model.hpp removed
    m_ImageIndex added. Vulkan error messages fixed
    second DrawMeshCommand added
    multiple objects added. Uniforms work with OpenGL but bugs with Vulkan
    multiple objects provided ( very bad code but it works )

### Problem
    -

## 14.03.2026
### Goal
    Create ECS component preview ( MeshComponent )
    Add DrawMeshCommand structure ( more information below )
    Add BeginFrame()/EndFrame() to IRenderer
    Review gpu resource manager's architecture

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - review GPU resource manager's architecture. It's confusing

    Maybe remove fe::resource:: namespace ?

    ```hpp
    struct SoundComponent {
    public:
        fe::pointer<resource::Sound> sound_id{};

        SoundComponent() = default;
        ~SoundComponent() = default;
    };

    struct MeshComponent {
    public:
        fe::pointer<resource::Mesh> mesh_id{}; // but there is no Mesh in fe::resource::

        MeshComponent() = default;
        ~MeshComponent() = default;
    };

    struct MeshHandle {
    public:
        fe::pointer<resource::Model> model_id{}; // + provide MeshHandle using to Resource Management ( both, CPU and GPU )
        uint32_t mesh_id{};

        MeshHandle() = default;
        ~MeshHandle() = default;
    };

    ///

    struct ScriptComponent {
    public:
        fe::pointer<resource::Script> script_id{}; // make script as a resource is a good idea, instead of using std::unique_ptr in DOD ECS

        ScriptComponent() = default;
        ~ScriptComponent() = default;
    };

    ///

    struct DrawMeshCommand { // add this thing for rendering
    public:
        fe::pointer<resource::Model> model{};
        uint32_t mesh_index = ~0; // ~0 means that renderer has to draw all meshes
    
        glm::mat4 transform{};

        DrawMeshCommand() = default;
        ~DrawMeshCommand() = default;
    };
    ```

### Done
    MeshComponent created
    Textures/Materials logic changed
    DrawMeshCommand provided
    GLTFImportContext added
    BeginFrame()/EndFrame() added

### Problem
    -

---

## 11.03.2026
### Goal
    Provide model loading to Vulkan
    Make camera movable
    ...

    Not now, but :
    - provide event system for platform
    - provide project "compiling" ( speed up )
    - speed up gLTF loading
    - review GPU resource manager's architecture. It's confusing

### Done
    Camera can move ( only showcase, works bad. 
        This needs to be done by the event system for platform )
    VKTypes.hpp -> VKRAII.hpp
    VKStructures.hpp -> VulkanTypes.hpp
    VKRAII.hpp -> VulkanRAII.hpp
    VulkanResourceManager configured
    bug fixed
    VulkanResourceManager done ( the model is blinking but it's drawing )
    Blinking fixed

### Problem
    -

---

## 10.03.2026
### Goal
    Make a CPU -> GPU model draw ( throw-in )
    Speed up gLTF loading
    Make camera movable
    ...

    Not now, but :
    - provide event system for platform
    - project "compiling"

### Done
    Model loading ( tinygltf -> Unified ) done without textures and materials
    OpenGL fix
    Model loaded correctly
    fe::pointer::packed() and fe::pointer::unpack() added
    almost done but it's crashing, I don't know why
    fixing the bug ( in progress )
    the bug fixed. do NEVER unbind EBO before unbinding VAO. EBO MUST BE UNBOUND ONLY AFTER VAO
     
### Problem
    -

---

## 09.03.2026
### Goal
    Create CPU primitive
    Create GPU primitive ( OpenGL )
    Make a CPU -> GPU model draw ( throw-in ) 
    ...
    Make camera movable

    Not now, but :
    - provide event system for platform

### Done
    Material added. Including problem - that's almost only thing I hate in C++
    Model added
    some fixes
    I have to include GLM to ForrGame
    it worked. Now there is a way to create GPU resources
    GPUResourceManager -> OpenGLResourceManager
    If the name in front of the word ( "OpenGL"ResourceManager ), when there is no interface ( no IResourceManager ),
        otherwise, if the name in back of the word ( Renderer"OpenGL" ), when there is an interface ( IRenderer )
    CPU/GPU Texture created
    CPU Primitive created
    MeshID removed
    fe::typed_pointer_storage::for_each became better
    VertexBuffers.hpp/cpp are removed
    Model loading from CPU to GPU done I guess
    "TODO : Create MeshImporter" added
    TextureImporter added
    StoreResource() -> ImportResource()
    GLTFImpoter added ( in progress )
    ResourceStorage::CreateResource() added
    typed_pointer_storage::create() improved
    tinygltf -> Unified for texture is done

### Problem
    -

---

## 08.03.2026
### Goal
    ...
    Load a gLTF module
    Make camera movable

    Not now, but :
    - provide event system for platform

### Done
    Happy Women's Day
    Bug fixed. Start point configured
    Prepared for model drawing
    tinygltf added
    bug fixed
    model drawn

    Found right now. You can do this :
    ```hpp
    template <template <typename> class C>
    struct Wrapper {};
    ```

### Problem
    -

---

## 07.03.2026
### Goal
    Add STB Image
    Load a texture with ResourceImporter
    UploadResource() -> StoreResource()
    ...
    Load a gLTF module
    Make camera movable

    Not now, but :
    - provide event system for platform

### Done
    UploadResource() -> StoreResource()
    STB Image added
    .clang-format-ignore added
    GetResource() added
    Texture adding added
    SetupGPUResources() -> CreateGPUResources()

    Found right now. You can do this :
    ```hpp
    template <template <typename> class C>
    struct Wrapper {};
    ```

### Problem
    -

---

## 06.03.2026
### Goal
    Load a gLTF module
    Make camera movable
    
    Not now, but :
    - provide event system for platform

    Provide DOD Resource Management System

### Done
    mutex removed from fe::typed_pointer_storage for now
    ResourceManager created
    fe::resource:: provided
    fe::resource::Texture and fe::resource::TextureMeta provided
    ResourceImporter created
    ResourceStorage created
    FORR_NODISCARDs added in pointer.hpp

### Problem
    School, lack of sleep

---

## 29.02.2026
### Goal
    Create Vulkan GPUResourceManager like in RendererOpenGL
    Load a gLTF module
    Make camera movable
    
    Not now, but :
    - provide event system for platform
    - remove errors when quit the application
    - remove errors when minimize the window ( will be done when provide platform system )

### Done
    VulkanResourceManager created
    RendererVulkan::VKRender() -> RendererVulkan::VKDraw()
    RendererVulkan::InitializeVertexBuffer() removed
    Triangle drawn
    VulkanResourceManager done
    erros when quit the application removed
    glFinish() calling added
    Shaders updated
    importing RenderMode and RenderIndexType
    Camera.hpp/Camera.cpp paths fixed
    Camera.hpp/Camera.cpp paths fixed ( again )
    GPUTypes.hpp now in Include/Forr/Graphics
    model dependencies imported except of Texture and Material

### Problem
    -

---

## 28.02.2026
### Goal
    Create VertexBuffer ( 1/2 )
	Create UniformBuffers
	Create DescriptorSetLayout
	Create DescriptorPool
	Create DescriptorSets
	Create Pipeline
    Create Camera
    Create RendererVulkan::windowResize()
    Create RendererVulkan::VKRender()
    ...
    Draw a triangle
    Create Vulkan GPUResourceManager like in RendererOpenGL

### Done
    vertex buffer ( index buffer ) done
    warning fixes
    queue submiting added ( was forgotten )
    ShaderData added to GPUTypes.hpp
    uniform buffers done
	descriptor set layout done
	descriptor pool done
	descriptor sets done
    VKGetQueueFamilyInfos() -> getQueueFamilyInfos()
    createShaderModule() created
    pipeline done
    Camera done
    InitializeCamera() added
    InitializeVulkan() -> InitializeBase()
    InitializeCamera() -> configureCamera()
    RendererVulkan::resizeWindow() added
    RendererVulkan::VKRender() added
    bug fixed

### Problem
    -

---

## 27.02.2026
### Goal
    Create RenderPass
	Create PipelineCache
	Create FrameBuffers
    Create VertexBuffer
	Create UniformBuffers
	Create DescriptorSetLayout
	Create DescriptorPool
	Create DescriptorSets
	Create Pipeline
    ...
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
    fe::Image -> fe::VulkanImage
    render pass done
    little fix
    unnecessary methods for swapchain creation removed
    pipeline cache done
    framebuffers done
    vertex buffer ( index buffer remaining )

    Remaining :
    - VertexBuffer ( 1/2 )
	- UniformBuffers
	- DescriptorSetLayout
	- DescriptorPool
	- DescriptorSets
	- Pipeline

### Problem
    -

---

## 26.02.2026
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

    Provide command buffers creation
    Create synchronization primitives
    Setup depth/stencil format

### Done
    Command buffers created
    Synchronization primitives created
    depth/stencil setting up done
    name cases fixed
    VKStructures.hpp added
    getMemoryType() added
    InitializeDepthStencil() done

### Problem
    -

---

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