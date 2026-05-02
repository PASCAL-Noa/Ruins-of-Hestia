#ifndef GPC_RENDER3D_SYSTEM_H
#define GPC_RENDER3D_SYSTEM_H

#include "Assets.h"
#include "ECS_Defines.h"
#include "System.h"
#include "Component.h"
#include "Geometry.h"
#include "Material.h"
#include "TransformComponents.h"

namespace GPC {

    struct RenderableComponent
    {
        Material RenderMaterial{};
        Geometry* pGeometry = nullptr;
        GraphicsProgram* pGraphicProgram = nullptr;
        bool      UseShadow = false;

        void SetTexture(std::string const& arrayName, std::string const& diffuse, std::string const& normal, std::string const& height);
        void SetTexture(std::string const& arrayName, std::string const& diffuse, std::string const& normal);
        void SetTexture(std::string const& arrayName, std::string const& diffuse);
    };

    struct Render3DComponent : public Component, public RenderableComponent {
        BIND_COMPONENT(RENDER3D);
        AUTO_COMPONENT_CONSTRUCTOR(Render3DComponent);
    };

    struct Batching3DComponent : public Component, public RenderableComponent {
        BIND_COMPONENT(BATCHING_3D);
        AUTO_COMPONENT_CONSTRUCTOR(Batching3DComponent);

        std::vector<Batched3DObject> Batched3DObjects {};
    };

    class Render3DSystem : public System {

        GPC::RenderObject3D         m_RenderContainer{};
        GPC::RenderObject3DBatched  m_RenderObjectBatched{};

    public:
        BIND_SYSTEM(RENDER3D);

        void OnUpdate() override;

        void Update3DObjects();
        void Update3DBatching();
    };
} // GPC

#endif // GPC_RENDER3D_SYSTEM_H
