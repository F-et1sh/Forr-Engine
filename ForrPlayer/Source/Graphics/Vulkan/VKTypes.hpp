/*===============================================

    Forr Engine

    File : VKTypes.cpp
    Role : Thin wrapper classes to provide RAII to Vulkan

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <span>

#include "Core/logging.hpp"
#include "Core/attributes.hpp"

#include <Volk/volk.h>

#include "Tools.hpp"

namespace fe::vk {
    struct Instance {
        Instance() = default;

        explicit Instance(VkInstance handle) noexcept : instance(handle) {}
        ~Instance() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(Instance)

        Instance(Instance&& other) noexcept : instance(other.instance) {
            other.instance = VK_NULL_HANDLE;
        }

        Instance& operator=(Instance&& other) noexcept {
            if (this != &other) {
                this->attach(other.instance);
                other.instance = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (instance) {
                vkDestroyInstance(instance, nullptr);
                instance = VK_NULL_HANDLE;
            }
        }

        void attach(VkInstance handle) noexcept {
            if (instance != handle) {
                this->reset();
                instance = handle;
            }
        }

        FORR_NODISCARD VkInstance get() const noexcept { return instance; }

        operator VkInstance() const noexcept { return instance; }

    private:
        VkInstance instance = VK_NULL_HANDLE;
    };

    struct Device {
        Device() = default;

        explicit Device(VkDevice handle) noexcept : device(handle) {}
        ~Device() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(Device)

        Device(Device&& other) noexcept : device(other.device) {
            other.device = VK_NULL_HANDLE;
        }

        Device& operator=(Device&& other) noexcept {
            if (this != &other) {
                this->attach(other.device);
                other.device = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (device) {
                vkDestroyDevice(device, nullptr);
                device = VK_NULL_HANDLE;
            }
        }

        void attach(VkDevice handle) noexcept {
            if (device != handle) {
                this->reset();
                device = handle;
            }
        }

        FORR_NODISCARD VkDevice get() const noexcept { return device; }

        operator VkDevice() const noexcept { return device; }

    private:
        VkDevice device = VK_NULL_HANDLE;
    };

} // namespace fe::vk
