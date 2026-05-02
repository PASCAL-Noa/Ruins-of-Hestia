#pragma once
#include <vector>
#include "Behaviour.h"

#define STATE_MACHINE_PAUSE -1

namespace GPC {

    template<typename T>
    class StateMachine
    {
        std::vector<Behaviour<T>*> m_Behaviours;
        int m_CurrentState;

        T* mp_Owner;

    public:
        StateMachine(T* pOwner, int stateCount);
        ~StateMachine();

        void Update();
        void SetState(int state);
        int GetCurrentState() const { return m_CurrentState; }

        Behaviour<T>* CreateBehaviour(int state);
    };

    template<typename T>
    StateMachine<T>::StateMachine(T* pOwner, int stateCount)
    {
        mp_Owner = pOwner;
        m_CurrentState = -1;
        m_Behaviours.resize(stateCount);
    }

    template<typename T>
    StateMachine<T>::~StateMachine()
    {
        for (Behaviour<T>* pBehaviour : m_Behaviours)
            delete pBehaviour;
    }


    template<typename T>
    void StateMachine<T>::SetState(int state)
    {
        if(m_CurrentState > 0 && m_CurrentState < m_Behaviours.size())
            m_Behaviours[m_CurrentState]->End();

        m_CurrentState = state;

        if(m_CurrentState == STATE_MACHINE_PAUSE)
            return;

        GPC_ASSERT(m_Behaviours[m_CurrentState]);

        m_Behaviours[m_CurrentState]->Start();
    }

    template<typename T>
    void StateMachine<T>::Update()
    {
        if(m_CurrentState == -1)
            return;

        int transitionState = m_Behaviours[m_CurrentState]->Update();

        if (transitionState == -1)
            return;

        SetState(transitionState);
    }

    template<typename T>
    Behaviour<T>* StateMachine<T>::CreateBehaviour(int state)
    {
        GPC_ASSERT(state >= 0 && state < m_Behaviours.size());

        Behaviour<T>* pBehaviour = new Behaviour<T>(mp_Owner);
        m_Behaviours[state] = pBehaviour;

        return pBehaviour;
    }
}
