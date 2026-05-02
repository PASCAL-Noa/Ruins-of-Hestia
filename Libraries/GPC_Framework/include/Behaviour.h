#pragma once
#include <vector>

namespace GPC  {
    template<typename T>
    class Action;

    template<typename T>
    class Transition;

    template<typename T>
    class Behaviour
    {
        std::vector<Action<T>*> m_Actions;
        std::vector<Transition<T>*> m_Transitions;
        T* mp_Owner;

    public:
        Behaviour(T* pOwner);
        ~Behaviour();

        void Start();
        int Update();
        void End();

        template<typename U>
        U* AddAction();

        Transition<T>* CreateTransition(int state);
    };

    template<typename T>
    Behaviour<T>::Behaviour(T* pOwner)
    {
        mp_Owner = pOwner;
    }

    template<typename T>
    Behaviour<T>::~Behaviour()
    {
        for (auto action : m_Actions)
        {
            delete action;
        }

        for (auto action : m_Transitions)
        {
            delete action;
        }
    }

    template<typename T>
    Transition<T>* Behaviour<T>::CreateTransition(int state)
    {
        Transition<T>* pTransition = new Transition<T>(state);
        m_Transitions.push_back(pTransition);

        return pTransition;
    }

    template<typename T>
    template<typename U>
    U* Behaviour<T>::AddAction()
    {
        static_assert(std::is_base_of_v<Action<T>, U>, "U must be derived from Action");

        U* pAction = new U();

        m_Actions.push_back(pAction);

        return pAction;
    }

    template<typename T>
    void Behaviour<T>::Start()
    {
        for (const auto& action : m_Actions)
        {
            action->Start(mp_Owner);
        }
    }

    template<typename T>
    int Behaviour<T>::Update()
    {
        for (const auto& transition : m_Transitions)
        {
            if (transition->Try(mp_Owner))
            {
                return transition->GetTransitionState();
            }
        }

        for (const auto& action : m_Actions)
        {
            action->Update(mp_Owner);
        }

        return -1;
    }


    template<typename T>
    void Behaviour<T>::End()
    {
        for (const auto& action : m_Actions)
        {
            action->End(mp_Owner);
        }
    }
}