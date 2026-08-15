/*===============================================

    Forr Engine

    File : RenderGraph.cpp
    Role : render graph and interface for render pass

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/RenderGraph.hpp"

fe::RenderGraphCompileResult fe::RenderGraph::Compile() {
    std::vector<CompiledRenderPass> render_passes{};
    render_passes.reserve(m_RenderPasses.size());

    // resource hash --> { resource version, previous state }
    std::unordered_map<fe::StringHash, Resource> resources_map{};

    // run Setup() for every added render pass and collect its required resources via fe::RenderGraphBuilder
    this->collectRenderPasses(render_passes, resources_map);

    // run Kahn's algorithm
    this->sortRenderPasses(render_passes);

    // run culling : remove unused render passes
    this->removeUnusedRenderPasses(render_passes, resources_map);
    // after culling 'resources_map' is broken, so we can't use it anymore
    resources_map.clear();

    // this is needed to translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass'
    std::vector<RenderPass> used_render_passes{};
    used_render_passes.reserve(render_passes.size());

    // translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass' and setup 'fe::RenderPass::compiled_image_barriers' and 'fe::RenderPass::compiled_buffer_barriers'
    this->translateRenderPasses(render_passes, resources_map, used_render_passes);

    m_RenderPasses.clear();
    m_RenderPasses = std::move(used_render_passes);

    std::unordered_map<fe::StringHash, ResourceLifetime> resource_lifetimes{};

    // setup resource lifetimes
    this->calculateResourceLifetimes(resource_lifetimes);

    struct ResourceInfo {
        bool   is_busy{};
        size_t storage_index{}; // this is a virtual storage index
    };
    // resource desc --> { is busy, storage index }
    std::unordered_map<render_graph::CreationCommand, std::vector<ResourceInfo>> resource_pool{};

    // this value is used to generate virtual storage indices
    size_t virtual_storage_index_number{};

    // returns resource virtual storage index
    auto acquire_resource_labmda = [&resource_pool, &virtual_storage_index_number](const render_graph::CreationCommand& desc) -> size_t {
        auto& pool_vector = resource_pool[desc];

        for (auto& resource_info : pool_vector) {
            if (!resource_info.is_busy) {

                resource_info.is_busy = true;
                return resource_info.storage_index;
            }
        }

        ResourceInfo& resource_info = pool_vector.emplace_back();
        resource_info.is_busy       = true;
        resource_info.storage_index = virtual_storage_index_number++;

        return resource_info.storage_index;
    };

    auto release_resource_lambda = [&resource_pool](const render_graph::CreationCommand& desc, size_t storage_index) {
        auto& pool_vector = resource_pool[desc];
        for (auto& resource_info : pool_vector) {
            if (resource_info.storage_index == storage_index)
                resource_info.is_busy = false;
        }
    };

    // hashed name --> resource desc
    std::unordered_map<fe::StringHash, render_graph::CreationCommand> hashed_to_desc_map{};
    hashed_to_desc_map.reserve(render_passes.size() * 5);
    for (const auto& render_pass : render_passes) {
        // images

        for (const auto& create_request : render_pass.image_create_requests) {
            fe::StringHash hashed_name      = create_request.handle.hashed_name;
            hashed_to_desc_map[hashed_name] = create_request;
        }

        // buffers

        for (const auto& create_request : render_pass.buffer_create_requests) {
            fe::StringHash hashed_name      = create_request.handle.hashed_name;
            hashed_to_desc_map[hashed_name] = create_request;
        }
    }

    // hashed name --> virtual storage index
    std::unordered_map<fe::StringHash, size_t> hashed_to_virtual_map{};

    // setup virtual indices
    this->setupVirtualIndices(acquire_resource_labmda,
                              release_resource_lambda,
                              resource_lifetimes,
                              hashed_to_desc_map,
                              hashed_to_virtual_map);

    RenderGraphCompileResult result{};
    result.image_descs.reserve(render_passes.size() * 5);
    result.buffer_descs.reserve(render_passes.size() * 5);

    for (CompiledRenderPass& render_pass : render_passes) {
        // images

        for (auto& create_request : render_pass.image_create_requests) {
            fe::StringHash hashed_name          = create_request.handle.hashed_name;
            create_request.handle.storage_index = hashed_to_virtual_map[hashed_name];
            result.image_descs.emplace_back(create_request);
        }

        // buffers

        for (auto& create_request : render_pass.buffer_create_requests) {
            fe::StringHash hashed_name          = create_request.handle.hashed_name;
            create_request.handle.storage_index = hashed_to_virtual_map[hashed_name];
            result.buffer_descs.emplace_back(create_request);
        }
    }

    for (size_t i = 0; i < m_RenderPasses.size(); i++) {
        RenderPass&         render_pass          = m_RenderPasses[i];
        CompiledRenderPass& compiled_render_pass = render_passes[i]; // using this index here will work

        render_graph::BeginRenderPass& begin_command = render_pass.compiled_begin_command;

        begin_command.is_clears_color   = true;
        begin_command.is_clears_depth   = true;
        begin_command.clear_color_value = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
        begin_command.clear_depth_value = 1.0f;

        if (!compiled_render_pass.image_writes.empty()) {
            auto it = hashed_to_desc_map.find(compiled_render_pass.image_writes[0].handle.hashed_name); // taking the first one
            if (it != hashed_to_desc_map.end()) {
                begin_command.viewport.extent = std::get<render_graph::ImageDesc>(it->second).extent;
            }
        }
        else if (compiled_render_pass.is_writes_to_screen) {
            begin_command.is_to_screen    = true;
            begin_command.viewport.extent = glm::ivec3(1920, 1080, 1); // TODO : use window's size
        }

        for (size_t j = 0; j < compiled_render_pass.image_writes.size(); j++) {
            if (j >= MAX_COLOR_ATTACHMENTS + 1) {              // plus one is depth stencil
                fe::logging::fatal("Too much render targets"); // TODO : rewrite this
            }

            CompiledRenderPass::ResourceBarrier& image_barrier = compiled_render_pass.image_writes[j];

            if (image_barrier.new_state == ResourceState::DEPTH_WRITE ||
                image_barrier.new_state == ResourceState::DEPTH_READ) {

                begin_command.depth_target     = image_barrier.handle.hashed_name;
                begin_command.has_depth_target = true;
            }
            else {
                begin_command.color_targets[j] = image_barrier.handle.hashed_name;
                begin_command.color_targets_count++;
            }
        }
    }

    //
    // Pass01 | read : []               , write :  ColorBuffer_v0
    // Pass02 | read :  ColorBuffer_v0  , write :  ColorBuffer_v1
    // Pass03 | read :  ColorBuffer_v1  , write :  OtherBuffer_v0
    // Pass04 | read :  OtherBuffer_v0  , write : OtherBuffer2_v0
    // Pass05 | read : OtherBuffer2_v0  , write :  ColorBuffer_v2
    // Pass06 | read :  ColorBuffer_v2  , write : OtherBuffer3_v0
    // Pass07 | read : OtherBuffer3_v0  , write :  ColorBuffer_v3
    //
    // ColorBuffer --> Backbuffer

    return result;
}

void fe::RenderGraph::SetupResourceBindings(const RenderGraphBindings& bindings) {
    for (RenderPass& render_pass : m_RenderPasses) {

        // images

        for (render_graph::ImageBarrier& image_barrier : render_pass.compiled_image_barriers) {
            auto it = bindings.image_bindings.find(image_barrier.handle.hashed_name);

            if (it != bindings.image_bindings.end()) {
                image_barrier.handle.storage_index = it->second;
            }
            else {
                fe::logging::error("Failed to set image barrier's index in fe::RenderGraph::SetupResourceBindings. Binding is missing for hash : %llu",
                                   image_barrier.handle.hashed_name);
            }
        }

        // buffers

        for (render_graph::BufferBarrier& buffer_barrier : render_pass.compiled_buffer_barriers) {
            auto it = bindings.buffer_bindings.find(buffer_barrier.handle.hashed_name);

            if (it != bindings.buffer_bindings.end()) {
                buffer_barrier.handle.storage_index = it->second;
            }
            else {
                fe::logging::error("Failed to set buffer barrier's index in fe::RenderGraph::SetupResourceBindings. Binding is missing for hash : %llu",
                                   buffer_barrier.handle.hashed_name);
            }
        }

        // color and depth/stencil targets

        render_graph::BeginRenderPass& begin_command = render_pass.compiled_begin_command;

        for (size_t i = 0; i < begin_command.color_targets_count; i++) {
            size_t& color_target = begin_command.color_targets[i];

            auto it = bindings.image_bindings.find(color_target);
            if (it != bindings.image_bindings.end()) {
                color_target = it->second;
            }
            else {
                fe::logging::error("Failed to set color target's texture index in fe::RenderGraph::SetupResourceBindings. Missing binding for color target hash : %llu",
                                   color_target);
            }
        }

        if (begin_command.has_depth_target) {
            auto it = bindings.image_bindings.find(begin_command.depth_target);
            if (it != bindings.image_bindings.end()) {
                begin_command.depth_target = it->second;
            }
            else {
                fe::logging::error("Failed to set depth target's texture index in fe::RenderGraph::SetupResourceBindings. Missing binding for depth target hash : %llu",
                                   begin_command.depth_target);
            }
        }
    }
}

fe::render_graph::CommandList fe::RenderGraph::Execute(const entt::registry& render_data) {
    render_graph::CommandList render_command_list{};
    render_command_list.reserve(m_RenderPasses.size() * 1024); // size in bytes

    for (const RenderPass& render_pass : m_RenderPasses) {
        RenderGraphContext context{ render_data };
        render_pass.execute_function(context, render_pass.mapped_data);

        for (const auto& barrier : render_pass.compiled_image_barriers)
            render_command_list.enqueue(barrier);

        for (const auto& barrier : render_pass.compiled_buffer_barriers)
            render_command_list.enqueue(barrier);

        render_command_list.enqueue(render_pass.compiled_begin_command);
        render_command_list.append_command_list(context.command_list);
        render_command_list.enqueue(render_pass.compiled_end_command);
    }

    return render_command_list;
}

void fe::RenderGraph::Clear() {
    for (auto& pass : m_RenderPasses) {
        if (pass.destroy_function && pass.mapped_data) {
            pass.destroy_function(pass.mapped_data);
        }
    }
    m_RenderPasses.clear();
    m_RenderPassesData.reset();
}

void fe::RenderGraph::collectRenderPasses(std::vector<CompiledRenderPass>&              render_passes_dst,
                                          std::unordered_map<fe::StringHash, Resource>& resources_map) {
    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{ m_ResourceManager };
        render_pass.setup_function(builder, render_pass.mapped_data);

        if (builder.image_writes.empty() &&
            builder.buffer_writes.empty() &&
            !builder.is_writes_to_screen) {

            builder.assertFatal("Render pass doesn't write to any image, buffer or to the screen");
        }

        // provide logging and remove render pass if there is a fatal error
        if (!builder.warnings.empty() ||
            !builder.errors.empty() ||
            !builder.fatals.empty()) {

            std::string all_warnings{ builder.warnings.empty() ? "no" : "" };
            std::string all_errors{ builder.errors.empty() ? "no" : "" };
            std::string all_fatals{ builder.fatals.empty() ? "no" : "" };

            // collect everything
            all_warnings.reserve(builder.warnings.size() * 50);
            for (const auto& message : builder.warnings) {
                all_warnings.append_range(message);
                all_warnings += '\n';
            }

            all_errors.reserve(builder.errors.size() * 50);
            for (const auto& message : builder.errors) {
                all_errors.append_range(message);
                all_errors += '\n';
            }

            all_fatals.reserve(builder.fatals.size() * 50);
            for (const auto& message : builder.fatals) {
                all_fatals.append_range(message);
                all_fatals += '\n';
            }

            if (!all_fatals.empty()) {
                fe::logging::error("RenderGraph : Failed to setup render pass %s. Render pass removed.\nWarnings : \n%s\n\nErrors : \n%s\n\nFatals : \n%s\n",
                                   render_pass.name.c_str(),
                                   all_warnings.c_str(),
                                   all_errors.c_str(),
                                   all_fatals.c_str());
                continue;
            }
            else {
                if (!all_warnings.empty())
                    fe::logging::warning("RenderGraph : Got a warnings while setting up render pass %s.\nWarnings : \n%s",
                                         render_pass.name.c_str(),
                                         all_warnings);
                if (!all_errors.empty())
                    fe::logging::error("RenderGraph : Got an errors while setting up render pass %s.\nErrors : \n%s",
                                       render_pass.name.c_str(),
                                       all_errors);
            }
        }

        auto& this_render_pass = render_passes_dst.emplace_back();

        this_render_pass.image_reads.reserve(builder.image_reads.size());
        this_render_pass.image_writes.reserve(builder.image_writes.size());

        // images

        for (const auto& read_barrier : builder.image_reads) {
            this_render_pass.image_reads.emplace_back(CompiledRenderPass::ResourceBarrier{ ResourceHandle{ read_barrier.handle.hashed_name, resources_map[read_barrier.handle.hashed_name].version },
                                                                                           resources_map[read_barrier.handle.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                           read_barrier.new_state });
            resources_map[read_barrier.handle.hashed_name].old_state = read_barrier.new_state;
        }

        for (const auto& write_barrier : builder.image_writes) {
            resources_map[write_barrier.handle.hashed_name].version++; // increasing version because while writing the resource is changes
            this_render_pass.image_writes.emplace_back(CompiledRenderPass::ResourceBarrier{ ResourceHandle{ write_barrier.handle.hashed_name, resources_map[write_barrier.handle.hashed_name].version },
                                                                                            resources_map[write_barrier.handle.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                            write_barrier.new_state });
            resources_map[write_barrier.handle.hashed_name].old_state = write_barrier.new_state;
        }

        // buffers

        for (const auto& read_barrier : builder.buffer_reads) {
            this_render_pass.buffer_reads.emplace_back(CompiledRenderPass::ResourceBarrier{ ResourceHandle{ read_barrier.handle.hashed_name, resources_map[read_barrier.handle.hashed_name].version },
                                                                                            resources_map[read_barrier.handle.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                            read_barrier.new_state });
            resources_map[read_barrier.handle.hashed_name].old_state = read_barrier.new_state;
        }

        for (const auto& write_barrier : builder.buffer_reads) {
            resources_map[write_barrier.handle.hashed_name].version++; // increasing version because while writing the resource is changes
            this_render_pass.buffer_reads.emplace_back(CompiledRenderPass::ResourceBarrier{ ResourceHandle{ write_barrier.handle.hashed_name, resources_map[write_barrier.handle.hashed_name].version },
                                                                                            resources_map[write_barrier.handle.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                            write_barrier.new_state });
            resources_map[write_barrier.handle.hashed_name].old_state = write_barrier.new_state;
        }

        this_render_pass.image_create_requests  = std::move(builder.image_create_requests);
        this_render_pass.buffer_create_requests = std::move(builder.buffer_create_requests);
        this_render_pass.name                   = render_pass.name;
        this_render_pass.is_writes_to_screen    = builder.is_writes_to_screen;
    }
}

void fe::RenderGraph::sortRenderPasses(std::vector<CompiledRenderPass>& render_passes_dst) {
    // a map, there every render pass refers to its dependencies
    std::vector<std::vector<uint32_t>> map(render_passes_dst.size());

    auto add_dependency_lambda = [&](uint32_t dependency_pass, uint32_t current_pass) {
        auto& dependencies = map[dependency_pass];
        if (dependencies.empty() || dependencies.back() != current_pass) {
            dependencies.emplace_back(current_pass);
        }
    };

    // resource handle --> render pass index
    std::unordered_map<ResourceHandle, uint32_t> last_writer{};
    // resource handle --> render passes indices
    std::unordered_map<ResourceHandle, std::vector<uint32_t>> current_readers{};

    for (size_t i = 0; i < render_passes_dst.size(); i++) {
        const auto& this_render_pass = render_passes_dst[i];

        // images

        for (const auto& read_barrier : this_render_pass.image_reads) {
            auto it = last_writer.find(read_barrier.handle);
            if (it != last_writer.end()) {
                add_dependency_lambda(it->second, i);
            }
            current_readers[read_barrier.handle].emplace_back(i);
        }

        for (const auto& write_barrier : this_render_pass.image_writes) {
            auto readers_it = current_readers.find(write_barrier.handle);

            if (readers_it != current_readers.end()) {
                for (uint32_t reader_pass : readers_it->second) {
                    if (reader_pass != i) {
                        add_dependency_lambda(reader_pass, i);
                    }
                }
                readers_it->second.clear();
            }

            auto writer_it = last_writer.find(write_barrier.handle);
            if (writer_it != last_writer.end()) {
                add_dependency_lambda(writer_it->second, i);
            }

            last_writer[write_barrier.handle] = i;
        }

        // buffers

        for (const auto& read_barrier : this_render_pass.buffer_reads) {
            auto it = last_writer.find(read_barrier.handle);
            if (it != last_writer.end()) {
                add_dependency_lambda(it->second, i);
            }
            current_readers[read_barrier.handle].emplace_back(i);
        }

        for (const auto& write_barrier : this_render_pass.buffer_writes) {
            auto readers_it = current_readers.find(write_barrier.handle);

            if (readers_it != current_readers.end()) {
                for (uint32_t reader_pass : readers_it->second) {
                    if (reader_pass != i) {
                        add_dependency_lambda(reader_pass, i);
                    }
                }
                readers_it->second.clear();
            }

            auto writer_it = last_writer.find(write_barrier.handle);
            if (writer_it != last_writer.end()) {
                add_dependency_lambda(writer_it->second, i);
            }

            last_writer[write_barrier.handle] = i;
        }
    }

    std::vector<uint32_t> indegree(render_passes_dst.size(), 0);

    for (uint32_t i = 0; i < render_passes_dst.size(); i++) {
        for (uint32_t j : map[i]) {
            indegree[j]++;
        }
    }

    std::queue<uint32_t> queue{};

    for (uint32_t i = 0; i < render_passes_dst.size(); i++) {
        if (indegree[i] == 0) {
            queue.push(i);
        }
    }

    std::vector<uint32_t> final_list{};
    final_list.reserve(render_passes_dst.size());

    while (!queue.empty()) {
        uint32_t top = queue.front();
        queue.pop();
        final_list.emplace_back(top);

        for (uint32_t j : map[top]) {
            indegree[j]--;

            if (indegree[j] == 0) queue.push(j);
        }
    }

    if (final_list.size() != render_passes_dst.size()) {
        fe::logging::fatal("Failed to compile RenderGraph : failed to run Kahn's algorithm - cycles found");
        return;
    }

    std::vector<CompiledRenderPass> sorted_render_passes{};
    sorted_render_passes.reserve(render_passes_dst.size());

    for (uint32_t i : final_list) {
        sorted_render_passes.emplace_back(std::move(render_passes_dst[i]));
    }

    render_passes_dst = std::move(sorted_render_passes);
}

void fe::RenderGraph::removeUnusedRenderPasses(std::vector<CompiledRenderPass>&              render_passes_dst,
                                               std::unordered_map<fe::StringHash, Resource>& resources_map) {
    if (render_passes_dst.empty()) return;

    std::vector<bool>                  is_pass_used(render_passes_dst.size(), false);
    std::unordered_set<ResourceHandle> used_resources{};

    for (size_t i = render_passes_dst.size() - 1; i >= 0; i--) {
        auto& render_pass = render_passes_dst[i];

        bool is_needed = render_pass.is_writes_to_screen;

        if (!is_needed) {
            is_needed = std::ranges::any_of(render_pass.image_writes, [&used_resources](const auto& barrier) { // images
                            return used_resources.contains(barrier.handle);
                        }) ||
                        std::ranges::any_of(render_pass.buffer_writes, [&used_resources](const auto& barrier) { // buffers
                            return used_resources.contains(barrier.handle);
                        });
        }

        if (is_needed) {
            is_pass_used[i] = true;

            // images

            for (const auto& read_barrier : render_pass.image_reads) {
                used_resources.insert(read_barrier.handle);
            }

            // buffers

            for (const auto& read_barrier : render_pass.buffer_reads) {
                used_resources.insert(read_barrier.handle);
            }
        }
    }

    std::vector<CompiledRenderPass> used_render_passes{};
    used_render_passes.reserve(render_passes_dst.size());

    for (size_t i = 0; i < render_passes_dst.size(); i++) {
        if (is_pass_used[i]) {
            used_render_passes.emplace_back(std::move(render_passes_dst[i]));
        }
    }

    render_passes_dst = std::move(used_render_passes);
}

void fe::RenderGraph::translateRenderPasses(std::vector<CompiledRenderPass>&              render_passes,
                                            std::unordered_map<fe::StringHash, Resource>& resources_map_dst,
                                            std::vector<RenderPass>&                      used_render_passes_dst) {
    for (const CompiledRenderPass& render_pass : render_passes) {
        // find this render pass in 'fe::RenderGraph::m_RenderPasses'
        auto it = std::ranges::find_if(m_RenderPasses, [&render_pass](const RenderPass& this_render_pass) -> bool {
            return render_pass.name == this_render_pass.name;
        });

        if (it != m_RenderPasses.end()) {
            it->compiled_image_barriers.reserve(render_pass.image_reads.size() + render_pass.image_writes.size());

            // images

            for (const CompiledRenderPass::ResourceBarrier& read_barrier : render_pass.image_reads) {
                auto& this_barrier = it->compiled_image_barriers.emplace_back();

                this_barrier.handle.hashed_name = read_barrier.handle.hashed_name;
                this_barrier.old_state          = resources_map_dst[this_barrier.handle.hashed_name].old_state;
                this_barrier.new_state          = read_barrier.new_state;

                resources_map_dst[this_barrier.handle.hashed_name].old_state = read_barrier.new_state;
            }

            for (const CompiledRenderPass::ResourceBarrier& write_barrier : render_pass.image_writes) {
                auto& this_barrier = it->compiled_image_barriers.emplace_back();

                this_barrier.handle.hashed_name = write_barrier.handle.hashed_name;
                this_barrier.old_state          = resources_map_dst[this_barrier.handle.hashed_name].old_state;
                this_barrier.new_state          = write_barrier.new_state;

                resources_map_dst[this_barrier.handle.hashed_name].old_state = write_barrier.new_state;
            }

            // buffers

            for (const CompiledRenderPass::ResourceBarrier& read_barrier : render_pass.buffer_reads) {
                auto& this_barrier = it->compiled_buffer_barriers.emplace_back();

                this_barrier.handle.hashed_name = read_barrier.handle.hashed_name;
                this_barrier.old_state          = resources_map_dst[this_barrier.handle.hashed_name].old_state;
                this_barrier.new_state          = read_barrier.new_state;

                resources_map_dst[this_barrier.handle.hashed_name].old_state = read_barrier.new_state;
            }

            for (const CompiledRenderPass::ResourceBarrier& write_barrier : render_pass.buffer_reads) {
                auto& this_barrier = it->compiled_buffer_barriers.emplace_back();

                this_barrier.handle.hashed_name = write_barrier.handle.hashed_name;
                this_barrier.old_state          = resources_map_dst[this_barrier.handle.hashed_name].old_state;
                this_barrier.new_state          = write_barrier.new_state;

                resources_map_dst[this_barrier.handle.hashed_name].old_state = write_barrier.new_state;
            }

            used_render_passes_dst.emplace_back(std::move(*it));
        }
    }
}

void fe::RenderGraph::calculateResourceLifetimes(std::unordered_map<fe::StringHash, ResourceLifetime>& resource_lifetimes) {
    resource_lifetimes.reserve(m_RenderPasses.size() * 5);
    for (size_t i = 0; i < m_RenderPasses.size(); i++) {
        const RenderPass& render_pass = m_RenderPasses[i];

        // images

        for (const render_graph::ImageBarrier& image_barrier : render_pass.compiled_image_barriers) {
            auto& lifetime = resource_lifetimes[image_barrier.handle.hashed_name];

            if (lifetime.first_pass_index == std::numeric_limits<uint32_t>::max()) {
                lifetime.first_pass_index = i;
            }

            lifetime.last_pass_index = i;
        }

        // buffers

        for (const render_graph::BufferBarrier& buffer_barrier : render_pass.compiled_buffer_barriers) {
            auto& lifetime = resource_lifetimes[buffer_barrier.handle.hashed_name];

            if (lifetime.first_pass_index == std::numeric_limits<uint32_t>::max()) {
                lifetime.first_pass_index = i;
            }

            lifetime.last_pass_index = i;
        }
    }
}
