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
    template <typename DestroyFn, typename HandleT = GLuint>
    class Handle {
    public:
        Handle() = default;
        explicit Handle(HandleT handle) noexcept : m_Handle(handle) {}

        ~Handle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(Handle)

        Handle(Handle&& other) noexcept : m_Handle(other.m_Handle) { other.m_Handle = 0; }

        Handle& operator=(Handle&& other) noexcept {
            if (this != &other) {
                this->attach(other.m_Handle);
                other.m_Handle = 0; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (m_Handle) {
                DestroyFn{}(m_Handle);
                m_Handle = 0;
            }
        }

        void attach(HandleT handle) noexcept {
            if (m_Handle != handle) {
                this->reset();
                m_Handle = handle;
            }
        }

        HandleT detach() noexcept {
            HandleT handle = m_Handle;
            m_Handle       = 0;
            return handle;
        }

        FORR_NODISCARD HandleT get() const noexcept { return m_Handle; }

        operator HandleT() const noexcept { return m_Handle; }

    protected:
        HandleT m_Handle{};
    };

    struct ShaderDestroy {
        void operator()(GLuint handle) const noexcept {
            glDeleteProgram(handle);
        }
    };

    struct VertexArrayDestroy {
        void operator()(GLuint handle) const noexcept {
            glDeleteVertexArrays(1, &handle);
        }
    };

    struct BufferDestroy {
        void operator()(GLuint handle) const noexcept {
            glDeleteBuffers(1, &handle);
        }
    };

    struct SyncDestroy {
        void operator()(GLsync handle) const noexcept {
            glDeleteSync(handle);
        }
    };

    struct TextureDestroy {
        void operator()(GLuint handle) const noexcept {
            GLuint64 resident_id = glGetTextureHandleARB(handle);

            if (glIsTextureHandleResidentARB(resident_id)) {
                glMakeTextureHandleNonResidentARB(resident_id);
            }

            glDeleteTextures(1, &handle);
        }
    };

    struct FramebufferDestroy {
        void operator()(GLuint handle) const noexcept {
            glDeleteFramebuffers(1, &handle);
        }
    };

    using ShaderProgram = Handle<ShaderDestroy>;
    using VertexArray   = Handle<VertexArrayDestroy>;
    using Buffer        = Handle<BufferDestroy>;
    using Sync          = Handle<SyncDestroy, GLsync>;
    using Texture       = Handle<TextureDestroy>;
    using Framebuffer   = Handle<FramebufferDestroy>;
} // namespace fe::gl
