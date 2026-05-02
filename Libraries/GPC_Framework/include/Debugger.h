#ifndef GPC_DEBUGGER_H
#define GPC_DEBUGGER_H

#include "Object.h"
#include <fstream>
#include <queue>

#include "GLM_Include.h"

#define GPC_SPACE GPC_INFO << ENDL

#ifdef NDEBUG
#define GPC_INFO	if(true) {} else GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::INFO) << "[INFO ] "
#define GPC_WARNING if(true) {} else GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::WARNING) << "[WARN ] "
#define GPC_ERROR	if(true) {} else GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::ERR).ReportError(__LINE__, __FILE__, __FUNCTION__) << "[ERROR] "
#else
#define GPC_INFO	GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::INFO) << "[INFO ] "
#define GPC_WARNING GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::WARNING) << "[WARN ] "
#define GPC_ERROR	GPC::Debugger::StartWithInputMode(GPC::DebuggerInputMode::ERR).ReportError(__LINE__, __FILE__, __FUNCTION__) << "[ERROR] "
#endif // NDEBUG

#define ENDL GPC::DBG_END_OF_LINE()

namespace GPC
{

	enum DebuggerOutput : int
	{
		CONSOLE			= 1,
		LOGS			= 2,
	};

	enum DebuggerInputMode : int
	{
		INFO		= 1,
		WARNING		= 2,
		ERR			= 4,
	};

	struct DebuggerDesc
	{
		int allowedInputMode = DebuggerInputMode::INFO | DebuggerInputMode::WARNING | DebuggerInputMode::ERR;
		int output = DebuggerOutput::CONSOLE | DebuggerOutput::LOGS;
	};

	struct DBG_END_OF_LINE {
		inline static constexpr char c = '\n';
	};

	GPC_INHERIT_OBJECT(Debugger)
	{
		inline static int m_Output;
		inline static int m_AllowedInputMode;
		inline static DebuggerInputMode m_CurrentInputMode;
		inline static std::ofstream m_LogFile;
		inline static bool m_IsInit = false;

		struct ErrorReport
		{
			unsigned int line;
			const char* file;
			const char* funcname;
			std::string msg;
		};
		inline static std::queue<ErrorReport> m_Errors;

		static Debugger& SelfRef();

		void Output(const std::string & str);

		void OutputError(const ErrorReport& report);
	public:

		static void Init(DebuggerDesc * pDesc);
		static Debugger& StartWithInputMode(DebuggerInputMode inputMode);
		static void Destroy();

		Debugger& ReportError(unsigned int line, const char* file, const char* funcname);

		Debugger& operator << (char c);
		Debugger& operator << (const char* str);
		Debugger& operator << (const std::string& str);
		Debugger& operator << (const std::string_view& str);

		Debugger& operator << (bool b);

		Debugger& operator << (const glm::vec2& v);
		Debugger& operator << (const glm::vec3& v);
		Debugger& operator << (const glm::vec4& v);
		Debugger& operator << (const glm::quat& q);

		Debugger& operator << (const GPC::Object& obj);

		void operator << (const DBG_END_OF_LINE& end);

		template<typename T> requires(!std::is_same_v<T, char> && (std::is_floating_point_v<T> || std::is_integral_v<T>))
		Debugger& operator << (T number) {
			Output(std::to_string(number));
			return *this;
		}

	};

}

#endif // GPC_DEBUGGER_H
