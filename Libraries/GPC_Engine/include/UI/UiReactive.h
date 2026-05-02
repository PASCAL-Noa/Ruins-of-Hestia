#pragma once

#include <cstring>
#include <string>
#include <type_traits>

#include "UI/UiSignal.h"

namespace GPC
{
    namespace UiDetail
    {
        template<typename T>
        concept Equatable = requires(const T& a, const T& b)
        {
            { a == b } -> std::convertible_to<bool>;
        };
    }

    template<typename T>
    class UiReactive
    {
    public:
        UiReactive() = default;
        explicit UiReactive(T value) : m_Value(std::move(value)) {}

        UiReactive(const UiReactive&) = delete;
        UiReactive& operator=(const UiReactive&) = delete;
        UiReactive(UiReactive&&) = default;
        UiReactive& operator=(UiReactive&&) = default;

        const T& Get() const { return m_Value; }
        operator const T&() const { return m_Value; }

        void Set(const T& value)
        {
            if constexpr (UiDetail::Equatable<T>)
            {
                if (!(m_Value == value))
                {
                    m_Value = value;
                    OnChanged.Emit(m_Value);
                }
            }
            else if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (std::memcmp(&m_Value, &value, sizeof(T)) != 0)
                {
                    m_Value = value;
                    OnChanged.Emit(m_Value);
                }
            }
            else
            {
                m_Value = value;
                OnChanged.Emit(m_Value);
            }
        }

        void SetSilent(const T& value)
        {
            m_Value = value;
        }

        void ForceNotify()
        {
            OnChanged.Emit(m_Value);
        }

        UiSignal<const T&> OnChanged;

    private:
        T m_Value{};
    };
}
