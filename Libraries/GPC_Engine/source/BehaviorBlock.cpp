#include "BehaviorBlock.h"

namespace GPC {
    BehaviorBlock::BehaviorBlock(EntityID eid) :
        m_EID(eid),
        m_BehaviorCount(0),
        mp_Behaviors()
    {
        for (uint32_t ebid = 0; ebid < MAX_BEHAVIOR_BY_ENTITY; ++ebid) {
            mp_Behaviors[ebid] = nullptr;
        }
    }

    void BehaviorBlock::DestroyBehavior(EntityBehaviorID ebid) {
        mp_Behaviors[ebid]->OnDestroy();
        delete mp_Behaviors[ebid];
    }

    EntityBehaviorID BehaviorBlock::GetNextBehaviorID() const {
        return m_BehaviorCount;
    }

    BehaviorBlock::~BehaviorBlock() {
        ClearBehaviors();
    }

    void BehaviorBlock::Start(const BehaviorCreateContext &ctx) const {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            if (mp_Behaviors[ebid]->IsStarted()) continue;
            mp_Behaviors[ebid]->OnStart(&ctx);
        }
    }

    void BehaviorBlock::Update(const BehaviorUpdateContext &ctx) const {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            if (mp_Behaviors[ebid]->IsEnabled())
                mp_Behaviors[ebid]->OnUpdate(&ctx);
        }
    }

    void BehaviorBlock::Collide3D(const BehaviorCollision3DContext *ctx) const {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            if (mp_Behaviors[ebid]->IsEnabled())
                mp_Behaviors[ebid]->OnCollision3D(ctx);
        }
    }

    void BehaviorBlock::Collide2D(const BehaviorCollision2DContext *ctx) const {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            if (mp_Behaviors[ebid]->IsEnabled())
                mp_Behaviors[ebid]->OnCollision2D(ctx);
        }
    }

    void BehaviorBlock::LateUpdate(const BehaviorUpdateContext &ctx) const {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            if (mp_Behaviors[ebid]->IsEnabled())
                mp_Behaviors[ebid]->OnLateUpdate(&ctx);
        }
    }

    void BehaviorBlock::RemoveBehavior(const Behavior *pBehavior) {
        RemoveBehavior(pBehavior->GetBehaviorID());
    }

    void BehaviorBlock::RemoveBehavior(EntityBehaviorID ebid) {
        // TODO : Check if this work correctly
        DestroyBehavior(ebid);
        mp_Behaviors[m_BehaviorCount - 1]->_SetEntityBehavior(ebid);
        mp_Behaviors[ebid] = mp_Behaviors[m_BehaviorCount - 1];
        m_BehaviorCount--;
    }

    void BehaviorBlock::ClearBehaviors() {
        for (EntityBehaviorID ebid = 0; ebid < m_BehaviorCount; ++ebid) {
            DestroyBehavior(ebid);
        }
        m_BehaviorCount = 0;
    }

    Behavior * BehaviorBlock::GetBehavior(EntityBehaviorID ebid) {
        GPC_ASSERT(ebid < m_BehaviorCount) {
            GPC_WARNING << "Trying to access a Behavior out of bounds ! For EID(" << m_EID << ")" << ENDL;
            return nullptr;
        }
        return mp_Behaviors[ebid];
    }

} // GPC