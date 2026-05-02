#pragma once

#include "Object.h"


namespace GPC
{

	template<typename Return_t, typename... Args>
	struct Callable {
		virtual Return_t Call(Args&&... args) = 0;
		virtual Return_t operator()(Args&&... args) = 0;
	};


	template<typename Return_t, typename... Args>
	GPC_INHERIT_OBJECT_RENAME(Function, "Function"), public Callable<Return_t, Args...>
	{
		using _func = Return_t(*)(Args...);

		_func mp_Func;

	public:
		Function(_func func) : mp_Func(func) {}
		virtual ~Function() = default;

		Return_t Call(Args&&... args) final
		{
			return mp_Func(std::forward<Args>(args)...);
		}

		Return_t operator()(Args&&... args) final
		{
			return mp_Func(std::forward<Args>(args)...);
		}

	};

	template<typename Obj_t, typename Return_t, typename... Args>
	GPC_INHERIT_OBJECT_RENAME(Method, "Method"), public Callable<Return_t, Args...>
	{
		using _func = Return_t(Obj_t::*)(Args...);

		_func mp_Func;
		Obj_t* mp_Obj;

	public:
		Method(Obj_t* pObj, _func func) : mp_Func(func), mp_Obj(pObj) {}
		virtual ~Method() = default;

		Return_t Call(Args&&... args) final
		{
			return (mp_Obj->*mp_Func)(std::forward<Args>(args)...);
		}


		Return_t operator()(Args&&... args) final
		{
			return (mp_Obj->*mp_Func)(std::forward<Args>(args)...);
		}

	};

	template<typename Return_t, typename... Args>
	GPC_INHERIT_OBJECT_RENAME(Lambda, "Lambda"), public Callable<Return_t, Args...>
	{
		using _func = Return_t(*)(void*, Args...);
		using _destroy = void(*)(void*);

		void* mp_Obj;
		_func mp_Func;
		_destroy mp_Destroy;

	public:
		template<typename F>
		Lambda(F&& f)
		{
			using Fun = std::decay_t<F>;
			mp_Obj = new Fun(std::forward<F>(f));
			mp_Func = +[](void* obj, Args... args) -> Return_t { return (*static_cast<Fun*>(obj))(args...); };
			mp_Destroy = +[](void* obj) -> void { delete static_cast<Fun*>(obj); };
		}

		virtual ~Lambda() { mp_Destroy(mp_Obj); }

		Return_t Call(Args&&... args) final
		{
			return mp_Func(mp_Obj, std::forward<Args>(args)...);
		}

		Return_t operator()(Args&&... args) final
		{
			return mp_Func(mp_Obj, std::forward<Args>(args)...);
		}


	};

	template<typename T>
	struct LambdaTraits;

	template<typename C, typename Ret, typename... Args>
	struct LambdaTraits<Ret(C::*)(Args...) const>
	{
		using Return = Ret;
		using Method = Lambda<Ret, Args...>;
	};

	
	template<typename Return_t, typename... Args>
	auto MakeCallable(Return_t(*func)(Args...))
	{
		using _func = Return_t(*)(Args...);
		return Function<Return_t, Args...>(std::forward<_func>(func));
	}

	template <typename Obj_t, typename Return_t, typename... Args >
	auto MakeCallable(Obj_t* pObj, Return_t(Obj_t::*func)(Args...))
	{
		using _func = Return_t(Obj_t::*)(Args...);
		return Method<Obj_t, Return_t, Args...>(pObj, std::forward<_func>(func));
	}

	template<typename F> 
	auto MakeCallable(F&& func)
	{
		using Decayed = std::decay_t<F>;
		using Traits = LambdaTraits<decltype(&Decayed::operator())>;
		return typename Traits::Method(std::forward<F>(func));
	}

	template<typename Return_t, typename... Args>
	auto MakeCallablePtr(Return_t(*func)(Args...))
	{
		using _func = Return_t(*)(Args...);
		return new Function<Return_t, Args...>(std::forward<_func>(func));
	}

	template <typename Obj_t, typename Return_t, typename... Args >
	auto MakeCallablePtr(Obj_t* pObj, Return_t(Obj_t::* func)(Args...))
	{
		using _func = Return_t(Obj_t::*)(Args...);
		return new Method<Obj_t, Return_t, Args...>(pObj, std::forward<_func>(func));
	}

	template<typename F>
	auto MakeCallablePtr(F&& func)
	{
		using Decayed = std::decay_t<F>;
		using Traits = LambdaTraits<decltype(&Decayed::operator())>;
		return new typename Traits::Method(std::forward<F>(func));
	}
}
