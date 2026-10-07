// RobAccess.h is licensed under the CC0 License unlike the rest of the project.
// Rob is possible thanks to: http://bloglitb.blogspot.com/2011/12/access-to-private-members-safer.html

#pragma once

/**
 * <b>USAGE INFO</b><br>
 * Define with <c>ROB_DEFINE_VAR</c><br>
 * Then, use <c>Obj->*RobAccess(class, var);</c>
 *
 * or
 *
 * Define with <c>ROB_DEFINE_FUNC(_CONST)</c><br>
 * Then, use <c>(Obj->*RobAccess(class, func))(params);</c>
 */
template<typename Tag, typename Tag::Type M>
struct Rob
{
	friend Tag::Type Access(Tag)
	{
		return M;
	}
};

#define ROB_DEFINE_VAR(_Class, _VarName, _VarType) \
	struct F##_Class##_VarName##Tag \
	{ \
		using Type = _VarType _Class::*; \
		friend Type Access(F##_Class##_VarName##Tag); \
	}; \
	template struct Rob<F##_Class##_VarName##Tag, &_Class::_VarName>

#define ROB_DEFINE_FUNC(_Class, _FuncName, _RetType, ...) \
	struct F##_Class##_FuncName##Tag \
	{ \
		using Type = _RetType (_Class::*)(__VA_ARGS__); \
		friend Type Access(F##_Class##_FuncName##Tag); \
	}; \
	template struct Rob<F##_Class##_FuncName##Tag, &_Class::_FuncName>

#define ROB_DEFINE_FUNC_CONST(_Class, _FuncName, _RetType, ...) \
	struct F##_Class##_FuncName##Tag \
	{ \
		using Type = _RetType (_Class::*)(__VA_ARGS__) const; \
		friend Type Access(F##_Class##_FuncName##Tag); \
	}; \
	template struct Rob<F##_Class##_FuncName##Tag, &_Class::_FuncName>

#define RobAccess(_Class, _MemberName) Access(F##_Class##_MemberName##Tag())