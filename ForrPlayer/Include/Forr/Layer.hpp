#pragma once

namespace fe {
    class ILayer {
    public:
        ILayer()          = default;
        virtual ~ILayer() = default;

        //virtual void OnEvent(Event& event) {}

        virtual void OnUpdate(float delta_time) {}
        virtual void OnRender() {}

    private:
    };
} // namespace fe
