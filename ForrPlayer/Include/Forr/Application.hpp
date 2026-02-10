#pragma once

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

    private:
    };
} // namespace fe
