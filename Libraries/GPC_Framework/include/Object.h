#ifndef GPC_OBJECT_H
#define GPC_OBJECT_H

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

#define GPC_INHERIT_OBJECT(name) class name : public GPC::_Object<#name>
#define GPC_INHERIT_OBJECT_RENAME(name, surname) class name : public GPC::_Object<surname>

#define GPC_OBJECT_COUNT (GPC::OBJECT_CREATED - GPC::OBJECT_DELETED)

namespace GPC
{
	inline static uint64_t OBJECT_CREATED = 0;
	inline static uint64_t OBJECT_DELETED = 0;

	template <std::size_t N>
	struct FixedString {
		char value[N];
		constexpr FixedString(const char(&str)[N]) {
			for (std::size_t i = 0; i < N; ++i)
				value[i] = str[i];
		}
	};

	struct Object
	{
		virtual ~Object() = default;

		virtual std::string_view GetName() const = 0;
		virtual void SetName(const std::string& name) = 0;
	};

	template<FixedString CLASS_NAME>
	class _Object : public Object
	{
#ifndef NDEBUG
		std::string m_FullName;
		uint64_t m_ID;
	public:
		constexpr _Object() : m_ID(OBJECT_CREATED++) { m_FullName = std::string("<") + CLASS_NAME.value + "::NO_NAME (ID=" + std::to_string(m_ID) + ")>"; }
		virtual ~_Object() { OBJECT_DELETED++; }
		[[nodiscard]] std::string_view GetName() const override final {
			return m_FullName;
		}
		void SetName(const std::string& name) override final {
			m_FullName = std::string("<") + CLASS_NAME.value + "::" + name + " (ID=" + std::to_string(m_ID) + ")>";
		}
#else
	public:
		constexpr _Object() {}
		[[nodiscard]] constexpr std::string_view GetName() const override final { return CLASS_NAME.value; }
		constexpr void SetName(const std::string& name) override final {}
#endif // !_NDEBUG
	};
}

#endif // GPC_OBJECT_H
