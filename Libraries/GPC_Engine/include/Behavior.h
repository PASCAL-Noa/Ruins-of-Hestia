#ifndef GPC_BEHAVIOR_H
#define GPC_BEHAVIOR_H

#include "Collision2DSystem.h"
#include "Collision3DSystem.h"
#include "ECS_Defines.h"
#include "CompileTimeHash.h"
#include "TransformComponents.h"

#define INHERIT_BEHAVIOR_CONSTRUCTOR(class)  \
explicit class(EntityID eid, EntityBehaviorID ebid) : Behavior(eid, ebid) {} \
inline static uint32_t TYPE = BEHAVIOR_HASH(class); \
uint32_t GetType() override { return BEHAVIOR_HASH(class); }

#define INHERIT_SPECIFIC_BEHAVIOR_CONSTRUCTOR(class, parent)  \
explicit class(EntityID eid, EntityBehaviorID ebid) : parent(eid, ebid) {} \
inline static uint32_t TYPE = BEHAVIOR_HASH(class); \
uint32_t GetType() override { return BEHAVIOR_HASH(class); }

#define BEHAVIOR_HASH(behavior) COMPILE_TIME_CRC32_STR(#behavior)

#define BEHAVIOR_COMPONENT(component_type) pCtx->pScene->GetComponent<component_type>(GetEntityID())

namespace GPC {
    class BehaviorBlock;
    class Scene;

    struct BehaviorUpdateContext {
        Scene* pScene;
        Clock* pClock;
    };

    struct BehaviorCreateContext {
        Scene* pScene;
        Clock* pClock;
    };

    struct BehaviorCollision3DContext {
        Scene* pScene;
        Clock* pClock;

        struct {
            EntityID EID;
            Collision3DComponent* pCollider;
            Transform3D* pTransform;
        } Collided;

        glm::vec3 CollisionPoint;
    };


    struct BehaviorCollision2DContext {
        Scene* pScene;
        Clock* pClock;

        struct {
            EntityID EID;
            Collision2DComponent* pCollider;
            Transform2D* pTransform;
        } Collided;

        glm::vec2 CollisionPoint;
    };

    using BehaviorType = uint32_t;

    class Behavior {
    private:
        EntityBehaviorID m_EBID;
        EntityID m_EID;
        bool m_IsEnabled;
        bool m_IsStarted = false;
    public:

        inline static uint32_t TYPE = BEHAVIOR_HASH(Behavior);
        virtual uint32_t GetType() = 0;
        explicit Behavior(EntityID eid, EntityBehaviorID ebid) : m_EID(eid), m_EBID(ebid), m_IsEnabled(true) {

        }
        virtual ~Behavior() = default;

        void _SetEntityBehavior(EntityBehaviorID ebid);

        [[nodiscard]] bool IsStarted() const { return m_IsStarted; }
        [[nodiscard]] bool IsEnabled() const { return m_IsEnabled; }
        [[nodiscard]] EntityID GetEntityID() const { return m_EID; }
        [[nodiscard]] EntityBehaviorID GetBehaviorID() const { return m_EBID; }

        void SetEnabled(bool active);

        /// Called when the behavior is Created
        virtual void OnCreate(const BehaviorCreateContext* pCtx) { }
        /// Called at the end of the frame when a new behavior is Created
        virtual void OnStart(const BehaviorCreateContext* pCtx) { m_IsStarted = true; }
        /// Called when the behavior is Destroyed
        virtual void OnDestroy() { }

        /// Called when the behavior is set to Enable
        virtual void OnEnable() { }
        /// Called when the behavior is set to Disable
        virtual void OnDisable() { }

        /// Called when the behavior is Updated
        virtual void OnUpdate(const BehaviorUpdateContext* pCtx) {  }
        /// Called when the behavior is Updated a second time
        virtual void OnLateUpdate(const BehaviorUpdateContext* pCtx) { }

        /// Called when the entity attached enter a collision
        virtual void OnCollision3D(const BehaviorCollision3DContext* pCtx) { }
        virtual void OnCollision2D(const BehaviorCollision2DContext* pCtx) { }

    };

    class BehaviorTest : public Behavior {
    public:
        INHERIT_BEHAVIOR_CONSTRUCTOR(BehaviorTest)

        void OnUpdate(const BehaviorUpdateContext* pCtx) override {
            GPC_INFO << "Update Test Behavior from entity(" << GetEntityID() << ")" << ENDL;
        }

    };
} // GPC

#endif // GPC_BEHAVIOR_H
