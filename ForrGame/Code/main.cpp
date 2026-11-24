/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrGame

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include <Forr/Core/dll_api.hpp>
#include <Forr/Core/context.hpp>
#include <Forr/Graphics/RenderGraph.hpp>

struct EmptryRenderPass {
    static void Setup(fe::RenderGraphBuilder& builder) {
    }

    static void Execute(fe::RenderGraphContext& context) {
    }
};

void on_initialize(fe::context& context) {
    context.get<fe::RenderGraph>().AddPass<EmptryRenderPass>("EmptyRenderPass from the game's dll");
}

void on_update(fe::context& context) {
}

void on_release(fe::context& context) {
}

extern "C" __declspec(dllexport) fe::dll_api::application_module* get_application_module() {
    static fe::dll_api::application_module result{
        "ForrGame-Example",
        true,
        &on_initialize,
        &on_update,
        &on_release
    };
    return &result;
}
