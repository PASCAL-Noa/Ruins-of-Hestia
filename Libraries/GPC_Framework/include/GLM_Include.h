#pragma once

#define GLM_FORCE_CONSTEXPR
#ifndef GLM_CONFIG_ALIGNED_GENTYPES
#define GLM_CONFIG_ALIGNED_GENTYPES GLM_ENABLE
#endif
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <glm/gtx/matrix_decompose.hpp>