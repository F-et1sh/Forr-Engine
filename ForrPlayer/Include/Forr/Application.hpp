/*===============================================

    Forr Engine

    File : Application.hpp
    Role : main class / almost everything in this class should be in 'main.cpp' of the actual game/user

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include <vector>

#include "Forr/Platform/IPlatformSystem.hpp"
#include "Forr/Graphics/IRenderer.hpp"
#include "Forr/Graphics/RenderGraph.hpp"
#include "Forr/ResourceManagement/ResourceManager.hpp"

#include "Forr/Graphics/DefaultRenderPasses.hpp"

#include "Forr/ECS/Components.hpp"           // temp
#include "Forr/ECS/Systems/RenderSystem.hpp" // temp

#include "Forr/Core/Events/EventBus.hpp"

#include "Forr/Core/guid.hpp"

namespace fe {
    struct ApplicationDesc {
        bool validation_enabled = true;

        GraphicsBackend graphics_backend{};
        PlatformBackend platform_backend{};

        std::vector<const char*> args{};

        std::string application_name{};
        WindowDesc  primary_window_desc{};

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };
    
    class ApplicationContext {
    public:
        template <typename T>
        void Register(T& system) {
            const TypeRegistry::TypeID type_id = TypeRegistry::get_id<T>();
            if (m_Types.find(type_id) != m_Types.end()) {
                fe::logging::fatal("You can't register a type twice. If you want to store some parameter variables, create a propper class-context and register it.\nType index was %llu", type_id);
            }
            m_Types[type_id] = static_cast<void*>(&system);
        }

        template<typename T>
        T& Get() {
            const TypeRegistry::TypeID type_id = TypeRegistry::get_id<T>();
            auto                       it      = m_Types.find(type_id);
            if (it == m_Types.end()) {
                fe::logging::fatal("Failed to find a type. Type wasn't registered.\nType index was %llu", type_id);
            }
            return *static_cast<T*>(it->second);
        }

    private:
        // the context doesn't store the types
        std::unordered_map<TypeRegistry::TypeID, void*> m_Types{};
    };

    class FORR_API Application { // TODO : move all logic of 'fe::Application' to 'main.cpp' of the game
    public:
        Application(const ApplicationDesc& desc);
        ~Application() = default;

        void Run();

    private:
        void InitializePlatformSystem(const ApplicationDesc& desc);
        void InitializeResourceManager(const ApplicationDesc& desc);
        void InitializePrimaryWindow(const ApplicationDesc& desc);
        void InitializeRenderer(const ApplicationDesc& desc);

    private:
        std::unique_ptr<IPlatformSystem> m_PlatformSystem{};
        std::unique_ptr<IRenderer>       m_Renderer{};
        std::unique_ptr<RenderGraph>     m_RenderGraph{};
        std::unique_ptr<ResourceManager> m_ResourceManager{};
        std::unique_ptr<EventBus>        m_EventBus{};

        size_t   m_PrimaryWindowID{};
        IWindow* m_PrimaryWindow{};

        ApplicationContext m_Context{};
    };
} // namespace fe
