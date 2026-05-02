#include "BehaviorBlockManager.h"

namespace GPC {
    BehaviorBlockManager::BehaviorBlockManager() :
        m_BehaviorBlocks()
    {
        for (EntityID eid = 0; eid < MAX_ENTITIES; ++eid) {
            m_BehaviorBlocks[eid] = BehaviorBlock(eid);
        }
    }

    BehaviorBlock* BehaviorBlockManager::GetBlock(EntityID eid) {
        return &m_BehaviorBlocks[eid];
    }

} // GPC