/*===============================================

    Forr Engine

    File : Camera.hpp
    Role : Abstract camera

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/attributes.hpp"

#pragma once

namespace fe {
    class Camera {
    public:
        enum Type { LOOKAT,
                    FIRSTPERSON };

    public:
        void setPerspective(float fov, float aspect, float znear, float zfar);
        void updateAspectRatio(float aspect);

        void                update(float delta_time);
        FORR_NODISCARD bool updatePad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time);

        FORR_NODISCARD bool  isMoving() const noexcept { return m_Keys.left || m_Keys.right || m_Keys.up || m_Keys.down; }
        FORR_NODISCARD float getNearClip() const noexcept { return m_ZNear; }
        FORR_NODISCARD float getFarClip() const noexcept { return m_ZFar; }

        FORR_NODISCARD glm::mat4 getPerspectiveMatrix() const noexcept { return m_PerspectiveMatrix; }
        FORR_NODISCARD glm::mat4 getViewMatrix() const noexcept { return m_ViewMatrix; }

        void setType(Type type) noexcept {
            this->m_Type = type;
        }

        void setRotationSpeed(float rotation_speed) noexcept {
            this->m_RotationSpeed = rotation_speed;
        }

        void setMovementSpeed(float movement_speed) noexcept {
            this->m_MovementSpeed = movement_speed;
        }

        void setPosition(glm::vec3 position) {
            this->m_Position = position;
            updateViewMatrix();
        }

        void setRotation(glm::vec3 rotation) {
            this->m_Rotation = rotation;
            updateViewMatrix();
        }

        void rotate(glm::vec3 delta) {
            this->m_Rotation += delta;
            updateViewMatrix();
        }

        void setTranslation(glm::vec3 translation) {
            this->m_Position = translation;
            updateViewMatrix();
        };

        void translate(glm::vec3 delta) {
            this->m_Position += delta;
            updateViewMatrix();
        }

    private:
        void updateViewMatrix();

    private:
        float m_Fov{};
        float m_ZNear{};
        float m_ZFar{};

        Type m_Type = Type::LOOKAT;

        glm::vec3 m_Position     = glm::vec3();
        glm::vec3 m_Rotation     = glm::vec3();
        glm::vec4 m_ViewPosition = glm::vec4();

        float m_MovementSpeed = 1.0f;
        float m_RotationSpeed = 1.0f;

        bool m_IsUpdated = true;
        bool m_FlipY     = false;

        glm::mat4 m_PerspectiveMatrix{};
        glm::mat4 m_ViewMatrix{};

        struct
        {
            bool left  = false;
            bool right = false;
            bool up    = false;
            bool down  = false;
        } m_Keys{};
    };
} // namespace fe
