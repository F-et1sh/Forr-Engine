/*===============================================

    Forr Engine

    File : Camera.cpp
    Role : Abstract camera

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Camera.hpp"

void fe::Camera::setPerspective(float fov, float aspect, float znear, float zfar) {
    glm::mat4 currentMatrix = m_PerspectiveMatrix;

    this->m_Fov   = fov;
    this->m_ZNear = znear;
    this->m_ZFar  = zfar;

    m_PerspectiveMatrix = glm::perspective(glm::radians(fov), aspect, znear, zfar);

    if (m_FlipY) {
        m_PerspectiveMatrix[1][1] *= -1.0f;
    }

    if (m_ViewMatrix != currentMatrix) {
        m_IsUpdated = true;
    }
}

void fe::Camera::updateAspectRatio(float aspect) {
    glm::mat4 current_matrix = m_PerspectiveMatrix;
    m_PerspectiveMatrix      = glm::perspective(glm::radians(m_Fov), aspect, m_ZNear, m_ZFar);

    if (m_FlipY) {
        m_PerspectiveMatrix[1][1] *= -1.0f;
    }

    if (m_ViewMatrix != current_matrix) {
        m_IsUpdated = true;
    }
}

void fe::Camera::update(float delta_time) {
    m_IsUpdated = false;

    if (m_Type == Type::FIRSTPERSON) {
        if (isMoving()) {
            glm::vec3 camera_front{};
            camera_front.x = -cos(glm::radians(m_Rotation.x)) * sin(glm::radians(m_Rotation.y));
            camera_front.y = sin(glm::radians(m_Rotation.x));
            camera_front.z = cos(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y));
            camera_front   = glm::normalize(camera_front);

            float moveSpeed = delta_time * m_MovementSpeed;

            if (m_Keys.up) m_Position += camera_front * moveSpeed;
            if (m_Keys.down) m_Position -= camera_front * moveSpeed;
            if (m_Keys.left) m_Position -= glm::normalize(glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            if (m_Keys.right) m_Position += glm::normalize(glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
        }
    }

    updateViewMatrix();
}

bool fe::Camera::updatePad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time) {
    bool result = false;

    if (m_Type == Type::FIRSTPERSON) {
        const float dead_zone = 0.0015f;
        const float range     = 1.0f - dead_zone;

        glm::vec3 camera_front;
        camera_front.x = -cos(glm::radians(m_Rotation.x)) * sin(glm::radians(m_Rotation.y));
        camera_front.y = sin(glm::radians(m_Rotation.x));
        camera_front.z = cos(glm::radians(m_Rotation.x)) * cos(glm::radians(m_Rotation.y));
        camera_front   = glm::normalize(camera_front);

        float moveSpeed = delta_time * m_MovementSpeed * 2.0f;
        float rotSpeed  = delta_time * m_RotationSpeed * 50.0f;

        // movement
        if (fabsf(axis_left.y) > dead_zone) {
            float pos = (fabsf(axis_left.y) - dead_zone) / range;
            m_Position -= camera_front * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            result = true;
        }
        if (fabsf(axis_left.x) > dead_zone) {
            float pos = (fabsf(axis_left.x) - dead_zone) / range;
            m_Position += glm::normalize(glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
            result = true;
        }

        // rotation
        if (fabsf(axis_right.x) > dead_zone) {
            float pos = (fabsf(axis_right.x) - dead_zone) / range;
            m_Rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
            result = true;
        }
        if (fabsf(axis_right.y) > dead_zone) {
            float pos = (fabsf(axis_right.y) - dead_zone) / range;
            m_Rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
            result = true;
        }
    }
    else {
        // TODO : look-at logic
    }

    if (result) updateViewMatrix();

    return result;
}

void fe::Camera::updateViewMatrix() {
    glm::mat4 current_matrix = m_ViewMatrix;

    glm::mat4 rotate_matrix = glm::mat4(1.0f);
    glm::mat4 transform_matrix{};

    rotate_matrix = glm::rotate(rotate_matrix, glm::radians(m_Rotation.x * (m_FlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
    rotate_matrix = glm::rotate(rotate_matrix, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotate_matrix = glm::rotate(rotate_matrix, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 translation = m_Position;
    if (m_FlipY) {
        translation.y *= -1.0f;
    }
    transform_matrix = glm::translate(glm::mat4(1.0f), translation);

    if (m_Type == Type::FIRSTPERSON) {
        m_ViewMatrix = rotate_matrix * transform_matrix;
    }
    else {
        m_ViewMatrix = transform_matrix * rotate_matrix;
    }

    m_ViewPosition = glm::vec4(m_Position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

    if (m_ViewMatrix != current_matrix) m_IsUpdated = true;
}
