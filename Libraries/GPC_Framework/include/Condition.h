#pragma once

namespace GPC  {
    template <typename T>
    class Transition;

    template<typename T>
    class Condition
    {
        bool Test(T* pOwner);

    public:
        virtual ~Condition() = default;

        bool Expected = true;

        virtual bool OnTest(T* pOwner) = 0;

        friend Transition<T>;
    };

    template<typename T>
    bool Condition<T>::Test(T* pOwner)
    {
        return Expected == OnTest(pOwner);
    }
}
