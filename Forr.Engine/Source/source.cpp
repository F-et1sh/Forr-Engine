#include "header.hpp"

void someclass::func() {
    fe::logging::debug("debug");
    fe::logging::info("info");
    fe::logging::warning("warning");
    fe::logging::error("error");
    fe::logging::fatal("fatal");
}