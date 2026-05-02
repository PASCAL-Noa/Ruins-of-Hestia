#pragma once

#include "Behavior.h"
#include "ECS_Defines.h"

namespace GPC {

    // TODO : si il y a des problèmes de performances au niveau des Behaviors => rendre continue en mémoire (mêmes behaviors)

    class BehaviorBlock {
        std::array<Behavior*, MAX_BEHAVIOR_BY_ENTITY> mp_Behaviors;

        uint8_t m_BehaviorCount;
        EntityID m_EID;

        void DestroyBehavior(EntityBehaviorID ebid);

        [[nodiscard]] EntityBehaviorID GetNextBehaviorID() const;
    public:

        BehaviorBlock() = default;
        BehaviorBlock(EntityID eid);
        virtual ~BehaviorBlock();

        void Start(const BehaviorCreateContext& ctx) const;
        void Update(const BehaviorUpdateContext& ctx) const;
        void Collide3D(const BehaviorCollision3DContext* ctx) const;
        void Collide2D(const BehaviorCollision2DContext* ctx) const;
        void LateUpdate(const BehaviorUpdateContext& ctx) const;

        template<typename Behavior_t>
        requires(std::is_base_of_v<Behavior, Behavior_t>)
        Behavior_t* AddBehavior(BehaviorCreateContext* pCtx);

        void RemoveBehavior(const Behavior* pBehavior);
        void RemoveBehavior(EntityBehaviorID ebid);
        void ClearBehaviors();

        [[nodiscard]] Behavior* GetBehavior(EntityBehaviorID ebid);

        template<typename Behavior_t>
        requires(std::is_base_of_v<Behavior, Behavior_t>)
        [[nodiscard]] Behavior_t* GetBehavior();
    };

    template<typename Behavior_t> requires(std::is_base_of_v<Behavior, Behavior_t>)
    Behavior_t* BehaviorBlock::AddBehavior(BehaviorCreateContext* pCtx) {
        EntityBehaviorID bid = GetNextBehaviorID();

        GPC_ASSERT(bid < MAX_BEHAVIOR_BY_ENTITY) {
            GPC_WARNING << "Behavior count exceeded max for EID(" << m_EID << ") !" << ENDL;
            return nullptr;
        }
        mp_Behaviors[bid] = new Behavior_t(m_EID, bid);
        m_BehaviorCount++;

        mp_Behaviors[bid]->OnCreate(pCtx);

        return static_cast<Behavior_t*>(mp_Behaviors[bid]);
    }

    template<typename Behavior_t> requires (std::is_base_of_v<Behavior, Behavior_t>)
    Behavior_t* BehaviorBlock::GetBehavior() {
        for (uint32_t i = 0; i < m_BehaviorCount; ++i) {
            if (Behavior_t::TYPE == mp_Behaviors[i]->GetType()) {
                return static_cast<Behavior_t*>(static_cast<void*>(mp_Behaviors[i]));
            }
        }
        return nullptr;
    }
} // GPC