// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/StringConv.h"
#include "Containers/UnrealString.h"
#include "String/FormatStringSan.h"

/**
 * Logs a Display message with the caller's [file:line] prefix.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_LOG_INFO(Msg, ...) \
	::ToroEngine::Logging::LogInfo(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Logs a Warning message with the caller's [file:line] prefix.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_LOG_WARNING(Msg, ...) \
	::ToroEngine::Logging::LogWarning(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Logs an Error message with the caller's [file:line] prefix without stopping play.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_LOG_ERROR(Msg, ...) \
	::ToroEngine::Logging::LogError(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Logs with the caller's [file:line] prefix through LogFatal.
 * Requests deferred PIE shutdown in the editor and terminates the game outside the editor.
 * Editor callers must exit their failing path safely after this macro returns.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_LOG_FATAL(Msg, ...) \
	::ToroEngine::Logging::LogFatal(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Reports with the caller's [file:line] prefix through ReportWarning.
 * Uses a Kismet warning when a script frame is active, or a native Warning log otherwise.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_REPORT_WARNING(Msg, ...) \
	::ToroEngine::Logging::ReportWarning(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Reports with the caller's [file:line] prefix through ReportError.
 * Uses a Kismet error when a script frame is active, or a native ensureAlwaysMsgf otherwise.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_REPORT_ERROR(Msg, ...) \
	::ToroEngine::Logging::ReportError(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Reports with the caller's [file:line] prefix through ReportFatal.
 * Requests deferred PIE shutdown in the editor and terminates the game outside the editor.
 * Editor callers must exit their failing path safely after this macro returns.
 * @param Msg A literal TEXT("...") printf-style format string.
 * @param ... Optional printf arguments matching Msg; pass TCHAR pointers for %s.
 */
#define TORO_REPORT_FATAL(Msg, ...) \
	::ToroEngine::Logging::ReportFatal(TEXT("[%s:%d] ") Msg, UTF8_TO_TCHAR(__FILE__), __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 * Formatted native logs and Blueprint execution diagnostics. Call these APIs on the game thread.
 * The TORO_LOG_* and TORO_REPORT_* macros add the caller's file and line to the message text so those are preferred.
 * Fatal diagnostics request deferred PIE shutdown in the editor and terminate games outside the editor.
 * @code{.cpp}
 * TORO_LOG_INFO(TEXT("Started"));
 * TORO_LOG_INFO(TEXT("Loaded %s: %d"), *Name, Count);
 * @endcode
 */
namespace ToroEngine::Logging
{
	namespace Private
	{
		TOROENGINE_API void LogInfo(const FString& Message);
		TOROENGINE_API void LogWarning(const FString& Message);
		TOROENGINE_API void LogError(const FString& Message);
		TOROENGINE_API void LogFatal(const FString& Message);
		TOROENGINE_API void ReportWarning(const FString& Message);
		TOROENGINE_API void ReportError(const FString& Message);
		TOROENGINE_API void ReportFatal(const FString& Message);
	}

	/**
	 * Writes a printf-formatted message to LogToroEngine at Display verbosity.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void LogInfo(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::LogInfo(FString::Printf(Format, Args...));
	}

	/**
	 * Writes a printf-formatted message to LogToroEngine at Warning verbosity.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void LogWarning(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::LogWarning(FString::Printf(Format, Args...));
	}

	/**
	 * Writes a printf-formatted message to LogToroEngine at Error verbosity without stopping play.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void LogError(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::LogError(FString::Printf(Format, Args...));
	}

	/**
	 * Logs an error and requests PIE shutdown in the editor; logs Fatal outside the editor.
	 * Editor calls return normally, even without an active session. The caller must exit its failing path safely.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void LogFatal(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::LogFatal(FString::Printf(Format, Args...));
	}

	/**
	 * Reports a printf-formatted Kismet warning when a script frame is active; otherwise logs a native Warning.
	 * A script frame remains active during synchronous native calls from Blueprint.
	 * @note Kismet warnings can terminate the process when launched with @code FATALSCRIPTWARNINGS @endcode.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void ReportWarning(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::ReportWarning(FString::Printf(Format, Args...));
	}

	/**
	 * Reports a printf-formatted Kismet error when a script frame is active; otherwise uses an ensure for stack trace.
	 * Native ensure reporting is attempted on every call in builds where ensure reporting is enabled.
	 * @note Kismet errors can terminate the process when launched with @code FATALSCRIPTWARNINGS @endcode.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void ReportError(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::ReportError(FString::Printf(Format, Args...));
	}

	/**
	 * Reports through ReportError and requests PIE shutdown in the editor; reports Kismet Fatal outside the editor.
	 * Editor calls return normally, even without an active session. The caller must exit its failing path safely.
	 * @note Kismet errors can still terminate the process when launched with @code FATALSCRIPTWARNINGS @endcode.
	 * @param Format Checked printf-style format string.
	 * @param Args Values substituted into Format.
	 */
	template<typename... Types>
	void ReportFatal(UE::Core::TCheckedFormatString<TCHAR, Types...> Format, Types... Args)
	{
		Private::ReportFatal(FString::Printf(Format, Args...));
	}
}
