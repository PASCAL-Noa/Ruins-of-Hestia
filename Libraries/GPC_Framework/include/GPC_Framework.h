#ifndef GPC_FRAMEWORK_H
#define GPC_FRAMEWORK_H

#include "vulkan/vulkan.h"


#include "Defines.h"
#include "Object.h"
#include "Debugger.h"
#include "ErrorType.h"
#include "Clock.h"
#include "VerboseLevel.h"
#include "WorkerThread.h"
#include "Callable.h"
#include "Transform.h"
#include "Action.h"
#include "Condition.h"
#include "Behaviour.h"
#include "Transition.h"
#include "StateMachine.h"
#include "openFbx/ofbx.h"

#ifndef NDEBUG
    #define DEBUG_ASSERT(expression, message) \
    do { \
    if (!(expression)) { \
    GPC_ERROR << "Assertion Failed: (" << #expression << ") | Message: " << message << ENDL; \
    assert(false); \
    } \
    } while (0)
#else
    #define DEBUG_ASSERT(expression, message) ((void)0)
#endif

namespace GPC
{
    // Maybe a déplacer ???
    template<typename O, typename I>
    static typename std::enable_if<!std::is_const<typename std::remove_pointer<O>::type>::value, O>::type pointer_cast(I pointer)
    {
        return static_cast<O>(static_cast<void*>(pointer));
    }

    constexpr float GENERAL_SCALE_EXPEDITION    = 10.0f;
    constexpr float GENERAL_SCALE_VILLAGE       = 10.0f;
}

#endif // GPC_FRAMEWORK_H
