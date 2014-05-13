#pragma once
//  (C) Copyright 2008-2014 Jonas Persson (l.j.persson@gmail.com)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  https://github.com/jonaspersson/utilib

#include "LogIf.hpp"
#include "Logger.hpp"
#ifdef ENSURE  // afx conflict
#undef ENSURE  
#endif
#if defined ENABLE_CONTRACTS || defined _DEBUG || defined DEBUG
// todo marker for return codes that needs to be handled. assert in debug build
#define TODO_HANDLE_FAIL(ok)  LOG_UNLESS_IMPL(ok,utilib::Logger("TodoHandleFailed"))
// verifies function precondition. assert in debug build
#define REQUIRE(ok) LOG_UNLESS_IMPL(ok,utilib::Logger("Require",utilib::Severity::Error,utilib::Setup::ShowLine|utilib::Setup::Callstack))
// verifies function postcondition. assert in debug build
#define ENSURE(ok) LOG_UNLESS_IMPL(ok,utilib::Logger("Ensure",utilib::Severity::Error))
// verifies expression that is supposted to hold. assert in debug build
#define CHECK(ok) LOG_UNLESS_IMPL(ok,utilib::Logger("Check",utilib::Severity::Error))
// verifies expression that is supposted to hold. assert in debug build
#define CHECK_BT(ok) LOG_UNLESS_IMPL(ok,utilib::Logger("Check",utilib::Severity::Error,utilib::Setup::ShowLine|utilib::Setup::Callstack)))

// logging
#define LOG_MSG(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Logg",utilib::Severity::Info))
#define LOG_IF(ok) LOG_IF_IMPL(ok,utilib::Logger("Logg",utilib::Severity::Info))
#define LOG_DEBUG(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Debug",utilib::Severity::Debug))
#define LOG_FATAL(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Fatal",utilib::Severity::Fatal))
#define LOG_ERROR(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Error",utilib::Severity::Error))
#define LOG_WARNING(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Warning",utilib::Severity::Warning))
#define PERF_WARN(msg) LOG_MESSAGE_IMPL(msg,utilib::Logger("Performance warning",utilib::Severity::Debug))
#define PERF_WARN_IF(ok) LOG_IF_IMPL(ok,utilib::Logger("Performance warning",utilib::Severity::Debug))

#else
#define TODO_HANDLE_FAIL(ok) LOG_NOOP
#define REQUIRE(ok) LOG_NOOP
#define ENSURE(ok) LOG_NOOP
#define CHECK(ok) LOG_NOOP
#define CHECK_BT(ok) LOG_NOOP
#define LOG_MSG(msg) LOG_NOOP
#define LOG_IF(ok) LOG_NOOP
#define LOG_DEBUG(msg) LOG_NOOP
#define LOG_FATAL(msg) LOG_NOOP
#define LOG_ERROR(msg) LOG_NOOP
#define LOG_WARNING(msg) LOG_NOOP
#define PERF_WARN(msg) LOG_NOOP
#define PERF_WARN_IF(msg) LOG_NOOP
#endif
