#pragma once
#include <memory>
#include <vector>
#include "Layer.hpp"

namespace fe {
    struct ApplicationDesc {
    public:
        int window_width  = 0;
        int window_height = 0;

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };

    class Application {
    public:
        Application() = default;
        Application(const ApplicationDesc& desc) { this->Initialize(desc); }

        ~Application() = default;

        void Release();
        void Initialize(const ApplicationDesc& desc);

        void Run();

        template <typename T> requires(std::is_base_of_v<ILayer, T>)
        inline void PushLayer() {
            m_LayerStack.push_back(std::make_unique<T>());
        }

    private:
        std::vector<std::unique_ptr<ILayer>> m_LayerStack;
    };
} // namespace fe
