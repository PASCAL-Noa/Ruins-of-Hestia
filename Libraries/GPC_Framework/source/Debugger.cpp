#include "Debugger.h"

namespace GPC
{
	Debugger& Debugger::SelfRef()
	{
		static Debugger dbg;
		return dbg;
	}

	void Debugger::Output(const std::string& str)
	{
		if (m_IsInit == false) return;
		if ((m_CurrentInputMode & m_AllowedInputMode) == false) return;
		if (m_Output & DebuggerOutput::CONSOLE)
		{
			std::cout << str;
		}
		if (m_CurrentInputMode == DebuggerInputMode::ERR) m_Errors.back().msg += str;
		if (m_Output & DebuggerOutput::LOGS)
		{
			m_LogFile << str;
		}
	}

	void Debugger::OutputError(const ErrorReport& report)
	{
		if (m_IsInit == false) return;
		StartWithInputMode(DebuggerInputMode::ERR);
		SelfRef() << "=================================\n";
		SelfRef() << "= Line     : " << report.line << '\n';
		SelfRef() << "= Filename : " << report.file << '\n';
		SelfRef() << "= Funcname : " << report.funcname << '\n';
		SelfRef() << "= msg      : \n" << report.msg;
		SelfRef() << "==================================" << ENDL;
	}

	void Debugger::Init(DebuggerDesc* pDesc)
	{
		if (m_IsInit) return;

		m_Output = pDesc->output;
		m_AllowedInputMode = pDesc->allowedInputMode;

		if (m_Output & DebuggerOutput::LOGS)
		{
			m_LogFile.open("Logs.txt", std::ios::binary);
		}

		m_IsInit = true;
	}

	Debugger& Debugger::StartWithInputMode(DebuggerInputMode inputMode)
	{
		Debugger& ref = Debugger::SelfRef();
		ref.m_CurrentInputMode = inputMode;
		return ref;
	}

	void Debugger::Destroy()
	{
		if (m_IsInit == false) return;

		while (m_Errors.size() > 0)
		{
			SelfRef().OutputError(m_Errors.front());
			m_Errors.pop();
		}

		if (m_Output & DebuggerOutput::LOGS)
		{
			m_LogFile.close();
		}

		m_IsInit = false;
	}

	Debugger& Debugger::ReportError(unsigned int line, const char* file, const char* funcname)
	{
		if (m_IsInit == false) return *this;
		m_Errors.push({
			.line = line,
			.file = file,
			.funcname = funcname
		});
		return *this;
	}

	Debugger& Debugger::operator<<(char c)
	{
		char str[2]{ c, '\0' };
		Output(str);
		return *this;
	}

	Debugger& Debugger::operator<<(const char* str)
	{
		Output(str);
		return *this;
	}

	Debugger& Debugger::operator<<(const std::string& str)
	{
		Output(str);
		return *this;
	}

	Debugger & Debugger::operator<<(const std::string_view &str) {
		Output(str.data());
		return *this;
	}

	Debugger & Debugger::operator<<(bool b) {
		static const std::string CHARACTERS[2] = { "TRUE", "FALSE" };
		Output(CHARACTERS[b ? 0 : 1]);
		return *this;
	}

	Debugger & Debugger::operator<<(const glm::vec2 &v) {
		return *this << "(" << v.x << ", " << v.y << ")";
	}

	Debugger & Debugger::operator<<(const glm::vec3 &v) {
		return *this << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	}

	Debugger & Debugger::operator<<(const glm::vec4 &v) {
		return *this << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	}

	Debugger & Debugger::operator<<(const glm::quat &q) {
		return *this << "(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
	}

	Debugger& Debugger::operator<<(const GPC::Object& obj)
	{
		Output(obj.GetName().data());
		return *this;
	}

	void Debugger::operator<<(const DBG_END_OF_LINE &end) {
		operator<<(end.c);
		if (m_Output & DebuggerOutput::CONSOLE)
			std::cout.flush();
		if (m_Output & DebuggerOutput::LOGS)
			m_LogFile.flush();
	}
}
