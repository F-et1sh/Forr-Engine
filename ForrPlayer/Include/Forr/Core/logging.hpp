/*===============================================

    Forr Engine

    File : logging.hpp
    Role : util for logging info to console.
        Based on Donut, MIT License ( https://github.com/NVIDIA-RTX/Donut )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <functional>

namespace fe::logging {

    enum class Severity {
        None = 0,
        Info,
        Debug,
        Warning,
        Error,
        Fatal
    };

    using Callback = std::function<void(Severity, char const*)>;

    void     setMinSeverity(Severity severity);
    void     setCallback(Callback func);
    Callback getCallback();
    void     resetCallback();

    // Windows : enables or disables future log messages to be shown as MessageBox'es.
    // This is the default mode.
    // Linux: no effect, log messages are always printed to the console.
    void EnableOutputToMessageBox(bool enable);

    // Windows : enables or disables future log messages to be printed to stdout or stderr, depending on severity.
    // Linux   : no effect, log messages are always printed to the console.
    void EnableOutputToConsole(bool enable);

    // Windows : enables or disables future log messages to be printed using OutputDebugString.
    // Linux   : no effect, log messages are always printed to the console.
    void EnableOutputToDebug(bool enable);

    // Windows : sets the caption to be used by the error message boxes.
    // Linux   : no effect.
    void setErrorMessageCaption(const char* caption);

    // Equivalent to the following sequence of calls:
    // - EnableOutputToConsole(true);
    // - EnableOutputToDebug(true);
    // - EnableOutputToMessageBox(false);
    void ConsoleApplicationMode();

    void message(Severity severity, const char* fmt, ...);
    void debug(const char* fmt, ...);
    void info(const char* fmt, ...);
    void warning(const char* fmt, ...);
    void error(const char* fmt, ...);
    void fatal(const char* fmt, ...);

} // namespace fe::logging
