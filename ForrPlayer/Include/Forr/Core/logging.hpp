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

    void     FORR_API setMinSeverity(Severity severity);
    void     FORR_API setCallback(Callback func);
    Callback FORR_API getCallback();
    void     FORR_API resetCallback();

    // Windows : enables or disables future log messages to be shown as MessageBox'es.
    // This is the default mode.
    // Linux: no effect, log messages are always printed to the console.
    void FORR_API EnableOutputToMessageBox(bool enable);

    // Windows : enables or disables future log messages to be printed to stdout or stderr, depending on severity.
    // Linux   : no effect, log messages are always printed to the console.
    void FORR_API EnableOutputToConsole(bool enable);

    // Windows : enables or disables future log messages to be printed using OutputDebugString.
    // Linux   : no effect, log messages are always printed to the console.
    void FORR_API EnableOutputToDebug(bool enable);

    // Windows : sets the caption to be used by the error message boxes.
    // Linux   : no effect.
    void FORR_API setErrorMessageCaption(const char* caption);

    // Equivalent to the following sequence of calls:
    // - EnableOutputToConsole(true);
    // - EnableOutputToDebug(true);
    // - EnableOutputToMessageBox(false);
    void FORR_API ConsoleApplicationMode();

    void FORR_API message(Severity severity, const char* fmt, ...);
    void FORR_API debug(const char* fmt, ...);
    void FORR_API info(const char* fmt, ...);
    void FORR_API warning(const char* fmt, ...);
    void FORR_API error(const char* fmt, ...);
    void FORR_API fatal(const char* fmt, ...);

} // namespace fe::logging
