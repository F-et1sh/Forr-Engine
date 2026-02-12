/*===============================================

    Forr Engine

    File : logging.cpp
    Role : util for logging info to console. 
        Based on Donut, MIT License ( https://github.com/NVIDIA-RTX/Donut )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Core/logging.hpp"

#include <cstdio>
#include <cstdarg>
#include <iterator>
#include <mutex>
#include <print>

#if _WIN32
#include <Windows.h>
#endif

namespace fe::logging {
    static constexpr size_t G_MESSAGE_BUFFER_SIZE   = 4096;
    static std::string      G_ERROR_MESSAGE_CAPTION = "Forr-Engine";

#if _WIN32
    static bool G_OUTPUT_TO_MESSAGE_BOX = true;
    static bool G_OUTPUT_TO_DEBUG       = true;
    static bool G_OUTPUT_TO_CONSOLE     = false;
#else
    static bool G_OUTPUT_TO_MESSAGE_BOX = false;
    static bool G_OUTPUT_TO_DEBUG       = false;
    static bool G_OUTPUT_TO_CONSOLE     = true;
#endif

    static std::mutex G_LOG_MUTEX;

    void DefaultCallback(Severity severity, const char* message) {
        const char* severity_text = "";

        // clang-format off
        switch (severity) {
            case Severity::Debug  : severity_text = "DEBUG"      ; break;
            case Severity::Info   : severity_text = "INFO"       ; break;
            case Severity::Warning: severity_text = "WARNING"    ; break;
            case Severity::Error  : severity_text = "ERROR"      ; break;
            case Severity::Fatal  : severity_text = "FATAL ERROR"; break;
            default: break;
        }
        // clang-format on

        char buf[G_MESSAGE_BUFFER_SIZE];
        snprintf(buf, std::size(buf), "%s : %s", severity_text, message);

        std::lock_guard<std::mutex> lock_guard(G_LOG_MUTEX);

#if _WIN32
        if (G_OUTPUT_TO_DEBUG) {
            OutputDebugStringA(buf);
            OutputDebugStringA("\n\n");
        }

        if (G_OUTPUT_TO_MESSAGE_BOX) {
            if (severity == Severity::Error || severity == Severity::Fatal) {
                MessageBoxA(0, buf, G_ERROR_MESSAGE_CAPTION.c_str(), MB_ICONERROR);
            }
        }

#endif
        if (G_OUTPUT_TO_CONSOLE) {
            if (severity == Severity::Error || severity == Severity::Fatal) {
                std::print(stderr, "{}\n\n", buf);
            }
            else {
                std::print(stdout, "{}\n\n", buf);
            }
        }

        if (severity == Severity::Fatal) {
            abort();
        }
    }

    void setErrorMessageCaption(const char* caption) {
        G_ERROR_MESSAGE_CAPTION = ((caption) != nullptr) ? caption : "";
    }

    static Callback G_CALLBACK     = &DefaultCallback;
    static Severity G_MIN_SEVERITY = Severity::Info;

    void setMinSeverity(Severity severity) {
        G_MIN_SEVERITY = severity;
    }

    void setCallback(Callback func) {
        G_CALLBACK = func;
    }

    Callback getCallback() {
        return G_CALLBACK;
    }

    void resetCallback() {
        G_CALLBACK = &DefaultCallback;
    }

    void EnableOutputToMessageBox(bool enable) {
        G_OUTPUT_TO_MESSAGE_BOX = enable;
    }

    void EnableOutputToConsole(bool enable) {
        G_OUTPUT_TO_CONSOLE = enable;
    }

    void EnableOutputToDebug(bool enable) {
        G_OUTPUT_TO_DEBUG = enable;
    }

    void ConsoleApplicationMode() {
        G_OUTPUT_TO_CONSOLE     = true;
        G_OUTPUT_TO_DEBUG       = true;
        G_OUTPUT_TO_MESSAGE_BOX = false;
    }

    void message(Severity severity, const char* fmt, ...) {
        if (static_cast<int>(G_MIN_SEVERITY) > static_cast<int>(severity)) {
            return;
        }

        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(severity, buffer);

        va_end(args);
    }

    void debug(const char* fmt, ...) {
        if (static_cast<int>(G_MIN_SEVERITY) > static_cast<int>(Severity::Debug)) {
            return;
        }

        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(Severity::Debug, buffer);

        va_end(args);
    }

    void info(const char* fmt, ...) {
        if (static_cast<int>(G_MIN_SEVERITY) > static_cast<int>(Severity::Info)) {
            return;
        }

        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(Severity::Info, buffer);

        va_end(args);
    }

    void warning(const char* fmt, ...) {
        if (static_cast<int>(G_MIN_SEVERITY) > static_cast<int>(Severity::Warning)) {
            return;
        }

        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(Severity::Warning, buffer);

        va_end(args);
    }

    void error(const char* fmt, ...) {
        if (static_cast<int>(G_MIN_SEVERITY) > static_cast<int>(Severity::Error)) {
            return;
        }

        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(Severity::Error, buffer);

        va_end(args);
    }

    void fatal(const char* fmt, ...) {
        char    buffer[G_MESSAGE_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, std::size(buffer), fmt, args);

        G_CALLBACK(Severity::Fatal, buffer);

        va_end(args);
    }

} // namespace fe::logging
