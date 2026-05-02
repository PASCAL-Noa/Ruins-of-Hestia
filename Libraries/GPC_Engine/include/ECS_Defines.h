#ifndef GPC_ECS_DEFINES_H
#define GPC_ECS_DEFINES_H

#include <bitset>
#include "GPC_Framework.h"

#define COMPONENT_MASK(type) (1 << static_cast<uint64_t>(GPC::ComponentType::type))
#define SYSTEM_MASK(type) (1 << static_cast<uint64_t>(GPC::SystemType::type))

namespace GPC {

    using EntityID = uint16_t;

    using EntityBehaviorID = uint8_t;
    constexpr uint32_t MAX_BEHAVIOR_BY_ENTITY = 50;

    enum class ComponentType : uint64_t
    {
        TRANSFORM3D   = 1,
        CAMERA        ,
        LIGHT         ,
        COLLISION3D   ,
        COLLISION2D   ,
        AUDIOSOURCE   ,
        AUDIOLISTENER ,
        RENDER3D      ,
        PARTICLE      ,
        TWEEN         ,
        ANIMATION     ,
        BATCHING_3D   ,
        UI_CANVAS    ,
        UI_TEXT      ,
        UI_SPRITE    ,
        UI_LAYOUT    ,
        UI_RAYCAST   ,
        UI_ANIM      ,
        COUNT
    };

    constexpr uint64_t MAX_COMPONENTS = static_cast<uint64_t>(GPC::ComponentType::COUNT) - 1;
    using Signature = std::bitset<MAX_COMPONENTS + 1>;

    enum class SystemType : uint64_t
    {
        RENDER3D        = 1,
        LIGHT           ,
        COLLISION3D     ,
        COLLISION2D     ,
        AUDIO           ,
        CAMERA          ,
        PARTICLE        ,
        TWEEN           ,
        ANIMATION       ,
        UI              ,
        COUNT
    };
    constexpr uint64_t MAX_SYSTEMS = static_cast<uint64_t>(GPC::SystemType::COUNT) - 1;

} // GPC

#endif // GPC_ECS_DEFINES_H
