#include "Behavior.h"

namespace GPC {
    void Behavior::_SetEntityBehavior(EntityBehaviorID ebid) {
        m_EBID = ebid;
    }

    void Behavior::SetEnabled(bool active) {
        m_IsEnabled = active;
        if (m_IsEnabled) {
            OnEnable();
        } else {
            OnDisable();
        }
    }

} // GPC