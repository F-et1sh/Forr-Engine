# Forr-Engine Not now, but :

# Added 14.04.2026
- provide capacity increasing for fe::typed_pointer_storage and use it in the engine

# Added 09.04.2026
- use submodules for shaderc, not from VulkanSDK

# Added 04.04.2026
- provide Assimp

# Added 02.04.2026
- create material instance

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