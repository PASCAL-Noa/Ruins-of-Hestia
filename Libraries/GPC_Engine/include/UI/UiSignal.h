#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "Callable.h"

namespace GPC
{
    using UiSignalHandle = uint32_t;

    template<typename... Args>
    class UiSignal
    {
    public:
        UiSignal() = default;

        ~UiSignal()
        {
            // Clear(); TODO Sigtrap
        }

        UiSignal(const UiSignal&) = delete;
        UiSignal& operator=(const UiSignal&) = delete;

        UiSignal(UiSignal&& other) noexcept
            : m_Slots(std::move(other.m_Slots))
            , m_NextHandle(other.m_NextHandle)
        {
            other.m_NextHandle = 1;
        }

        UiSignal& operator=(UiSignal&& other) noexcept
        {
            if (this != &other)
            {
                Clear();
                m_Slots = std::move(other.m_Slots);
                m_NextHandle = other.m_NextHandle;
                other.m_NextHandle = 1;
            }
            return *this;
        }

        UiSignalHandle Connect(Callable<void, Args...>* pSlot)
        {
            if (!pSlot) return 0;

            const UiSignalHandle handle = m_NextHandle++;
            m_Slots.push_back({ handle, pSlot, true });
            return handle;
        }

        template<typename Lambda_t>
        UiSignalHandle ConnectLambda(Lambda_t&& lambda)
        {
            return Connect(MakeCallablePtr(std::forward<Lambda_t>(lambda)));
        }

        bool Disconnect(UiSignalHandle handle)
        {
            for (auto& slot : m_Slots)
            {
                if (slot.Handle == handle && slot.Alive)
                {
                    slot.Alive = false;
                    if (slot.Owned && slot.pCallable)
                    {
                        delete slot.pCallable;
                        slot.pCallable = nullptr;
                    }
                    return true;
                }
            }
            return false;
        }

        void Clear()
        {
            for (auto& slot : m_Slots)
            {
                if (slot.Alive && slot.Owned && slot.pCallable)
                {
                    delete slot.pCallable;
                }
                slot.Alive = false;
                slot.pCallable = nullptr;
            }
            m_Slots.clear();
        }

        void Emit(Args... args)
        {
            bool hasDead = false;
            for (auto& slot : m_Slots)
            {
                if (slot.Alive && slot.pCallable)
                {
                    slot.pCallable->Call(std::forward<Args>(args)...);
                }
                if (!slot.Alive) hasDead = true;
            }
            if (hasDead) Compact();
        }

        size_t Count() const
        {
            size_t alive = 0;
            for (const auto& slot : m_Slots)
            {
                if (slot.Alive) ++alive;
            }
            return alive;
        }

    private:
        struct Slot
        {
            UiSignalHandle     Handle   = 0;
            Callable<void, Args...>* pCallable = nullptr;
            bool                Alive    = false;
            bool                Owned    = true;
        };

        void Compact()
        {
            m_Slots.erase(
                std::remove_if(m_Slots.begin(), m_Slots.end(),
                    [](const Slot& s) { return !s.Alive; }),
                m_Slots.end());
        }

        std::vector<Slot> m_Slots;
        UiSignalHandle   m_NextHandle = 1;
    };
}
