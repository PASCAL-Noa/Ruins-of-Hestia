#ifndef GPC_BEHAVIOR_BLOCK_MANAGER_H
#define GPC_BEHAVIOR_BLOCK_MANAGER_H

#include "ECS_Defines.h"
#include "BehaviorBlock.h"

namespace GPC {

    class BehaviorBlockManager {
        std::array<BehaviorBlock, MAX_ENTITIES> m_BehaviorBlocks;
    public:
        BehaviorBlockManager();
        virtual ~BehaviorBlockManager() = default;

        [[nodiscard]] BehaviorBlock* GetBlock(EntityID eid);
    };
} // GPC

#endif // GPC_BEHAVIOR_BLOCK_MANAGER_H
