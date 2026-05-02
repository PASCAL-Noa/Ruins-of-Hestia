#include "DebugBehavior/CollisionTestDebug.h"

namespace GPC {
    void CollisionTestDebug::OnCollision3D(const BehaviorCollision3DContext *pCtx) {
        Behavior::OnCollision3D(pCtx);
        GPC_INFO << "Entity(" << GetEntityID() << ") Collided Entity(" << pCtx->Collided.EID <<")" << ENDL;
    }
} // GPC