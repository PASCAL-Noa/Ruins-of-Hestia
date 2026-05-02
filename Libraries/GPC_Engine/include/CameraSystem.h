#pragma once

#include "ECS_Defines.h"
#include "Component.h"
#include "RenderWindow.h"
#include "System.h"

namespace GPC {

    struct CameraComponent : public Component {
        BIND_COMPONENT(CAMERA);
        AUTO_COMPONENT_CONSTRUCTOR(CameraComponent);

        CameraContext Context{};
        bool Active = true;

        bool IsUsingOrthographic() const { return Context.UseOrthographic; }
        void UseOrthographic(float size = 1.5f) { Context.UseOrthographic = true; Context.Height = size; }
        void UsePerspective(Radian FOV = glm::radians(70.0f)) { Context.UseOrthographic = false; Context.FOV = FOV; }
    };

    class CameraSystem : public System {
        unsigned int m_CurrentCamera = 0;

        void UpdateCurrentActive();

    public:
        BIND_SYSTEM(CAMERA);
        CameraSystem() = default;

        CameraComponent* CameraActive();

        void OnUpdate() override;

    };

} // GPC
