#ifndef GPC_DEFINES_H
#define GPC_DEFINES_H

#include <cstdint>
#include "GLM_Include.h"

namespace GPC
{
	constexpr float		GPC_PI					= glm::pi<float>();

    constexpr uint64_t	MAX_ENTITIES			= 8'192;
	constexpr uint8_t	MAX_GENERATED_FRAME		= 2; //TODO MAYBE DELETED BECAUSE HAVE TO MATCH SWAPCHAIN FRAMES
	constexpr uint32_t	MAX_LIGHTS				= 50;
	constexpr uint32_t	MAX_SHADOWS_LIGHT		= 50;
	constexpr uint32_t	MAX_POINT_LIGHT			= 16;
	constexpr uint32_t	MAX_PARTICLES			= 1'048'576;
	// constexpr uint32_t	MAX_PARTICLES			= 1'024;
	using Radian =  float;
	using Degree =  float;

	constexpr glm::vec4 CLEAR_COLOR = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr uint32_t  CCACHE_REBUILD_TEST = 1;
}

#endif // GPC_DEFINES_H
