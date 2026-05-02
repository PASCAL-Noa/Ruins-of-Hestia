#ifndef GPC_TRANSFORM_COMPONENTS_H
#define GPC_TRANSFORM_COMPONENTS_H

#include "ECS_Defines.h"
#include "Transform.h"
#include "Component.h"

namespace GPC {

    struct Transform3D : public Component {
        BIND_COMPONENT(TRANSFORM3D);
        AUTO_COMPONENT_CONSTRUCTOR(Transform3D);
        ~Transform3D() override;

        static inline bool s_IsShuttingDown = false;
        Transform LocalTransform;

        [[nodiscard]] const glm::mat4x4& GetWorldMatrix();
        [[nodiscard]] glm::mat4x4 GetWorldRotation();
        [[nodiscard]] glm::vec3 GetWorldPosition();

        [[nodiscard]] Transform3D* GetParent();
        [[nodiscard]] Transform3D* GetChild(uint32_t index);
        [[nodiscard]] const Transform3D* GetParent() const;
        [[nodiscard]] const Transform3D* GetChild(uint32_t index) const;
        [[nodiscard]] uint32_t GetChildCount() const;
        void SetParent(Transform3D* pParent);
        void AddChild(Transform3D* pChild);
        void RemoveParent();
        void RemoveChild(uint32_t index);
    private:

        void UpdateLocalWithWorldParentRemoval();
        void UpdateLocalWithWorldParentAddition();
        void UpdateWorldMatrix();
        void SetDirty();
        glm::mat4x4 m_World{1.0f};
        Transform3D* mp_Parent = nullptr;
        std::vector<Transform3D*> mp_Childs{};
        bool m_IsDirty = true;

        friend class Transform;
    };

    using Transform2D = Transform3D; // TODO Si jamais on a le temps faire un transform 2D complet mais pour le moment on utilise les meme pour les deux
} // GPC

#endif // GPC_TRANSFORM_COMPONENTS_H
