#pragma once

namespace GPC  {
    template<typename T>
    class Action
    {
    public:
        virtual ~Action() = default;

        virtual void Start(T* pOwner) = 0;
        virtual void Update(T* pOwner) = 0;
        virtual void End(T* pOwner) = 0;
    };
}
