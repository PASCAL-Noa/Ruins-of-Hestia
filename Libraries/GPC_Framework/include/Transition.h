#pragma once
#include <vector>
#include "GPC_Framework.h"

namespace GPC  {
    template<typename T>
    class Condition;

    template<typename T>
    class Transition
    {
    protected:
        std::vector<Condition<T>*> m_Conditions;
        int m_TransitionState;

    public:
        Transition(int transitionState) : m_TransitionState(transitionState) {}

        template<typename U>
        U* AddCondition();

        bool Try(T* pOwner);
        int GetTransitionState() { return m_TransitionState; }
    };

    template<typename T>
    template<typename U>
    U* Transition<T>::AddCondition()
    {
        static_assert(std::is_base_of_v<Condition<T>, U>, "U must be derived from Condition");

        U* pCondition = new U();

        m_Conditions.push_back(pCondition);

        return pCondition;
    }

    template<typename T>
    bool Transition<T>::Try(T* pOwner)
    {
        for (const auto& condition : m_Conditions)
        {
            if (condition->Test(pOwner) == false)
                return false;
        }

        return true;
    }
}