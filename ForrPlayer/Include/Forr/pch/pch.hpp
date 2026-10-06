/*===============================================

    Forr Engine

    File : pch.hpp
    Role : PCH

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <unordered_set>
#include <array>
#include <functional>
#include <map>
#include <queue>
#include <span>
#include <vector>
#include <concepts>
#include <variant>
#include <expected>
#include <utility>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "entt/entt.hpp"

#pragma warning(disable: 4251) // dllexport/dllimport warning