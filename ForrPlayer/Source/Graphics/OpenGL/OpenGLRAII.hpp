/*===============================================

    Forr Engine

    File : OpenGLRAII.cpp
    Role : Thin wrapper classes to provide RAII to OpenGL.
        fe::gl:: - is a namespace that contains only thin wrappers.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include <glad/gl.h>

namespace fe::gl {
    template <typename DestroyFn>
    class OpenGLHandle {
    public:
        OpenGLHandle() = default;
        explicit OpenGLHandle(GLuint handle) noexcept : handle(handle) {}

        ~OpenGLHandle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(OpenGLHandle)

        OpenGLHandle(OpenGLHandle&& other) noexcept : handle(other.handle) {
            other.handle = 0;
        }

        OpenGLHandle& operator=(OpenGLHandle&& other) noexcept {
            if (this != &other) {
                this->attach(other.device, other.handle);
                other.handle = 0; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (handle) {
                DestroyFn{}(handle);

                handle = 0;
            }
        }

        void attach(GLuint handle) noexcept {
            if (this->handle != handle) {

                this->reset();

                this->handle = handle;
            }
        }

        FORR_NODISCARD GLuint get() const noexcept { return handle; }
                              operator GLuint() const noexcept { return handle; }

    protected:
        GLuint handle{};
    };

    struct ShaderDestroy {
        void operator()(GLuint handle) const noexcept {
            glDeleteProgram(handle);
        }
    };

    using ShaderProgram = OpenGLHandle<ShaderDestroy>;
} // namespace fe::gl
