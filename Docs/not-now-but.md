# Forr-Engine Not now, but :

# Added 29.08.2026
- make a video for YouTube / start this task at 7th September

# Added 23.08.2026
- rework shader system : provide flexible API for user to manage Slang specialization
- remove resource manager and renderer from 'fe::render_graph::RenderGraphBuilder'
- setup rendering, stabilize the system
- provide same rendering for Vulkan
- fix Vukan VMA error
- provide debug tools with Dear ImGui
- fix window resizing for GL/VK
- do not create GPU resources if they're already created --> InitializeGPUResources()
- fix double loading for resource management
- provide string_pool and use std::string_view instead of always using std::string - helps to decrease allocations
- provide std::expected<> in fe::ResourceManager and use it in all new modules
- provide logging macro to log current function automatically

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 20.08.2026
- provide user-custom generics specialization for 'fe::SlangParser'

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 17.07.2026
### Started at 19.08.2026
- migrate on std::expected ( SlangParser, ... )

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 06.07.2026
- add OpenGL Legacy backend

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 12.05.2026
### DONE 23.08.2026 ( it was done much earlier, but I'm changing its status only this day )
- use AoS instead of SoA

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 30.04.2026
### DONE 06.07.2026 ( it was done much earlier, but I'm changing its status only this day )
- provide sorting and passing meshes aka draw commands, not hierarchy-based system like now.
    "Tea in a cup does NOT belong to that cup - it is a separate object"

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 14.04.2026
- provide capacity increasing for fe::typed_pointer_storage and use it in the engine

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 09.04.2026
### DONE 23.08.2026 ( it was done much earlier, but I'm changing its status only this day ) ( I'm not using shaderc anymore )
- use submodules for shaderc, not from VulkanSDK

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 04.04.2026
- provide Assimp

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 02.04.2026
### DONE 23.08.2026 ( it was done much earlier, but I'm changing its status only this day )
- create material instance

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">

# Added 23.03.2026
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

<hr style="height: 2px; background-color: #555; border: none; margin: 30px 0;">