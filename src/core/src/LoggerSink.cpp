#include "cesium/omniverse/LoggerSink.h"

namespace cesium::omniverse {
void LoggerSink::sink_it_([[maybe_unused]] const spdlog::details::log_msg& msg) {
    // The reason we don't need to provide a log channel as the first argument to each of these OMNI_LOG_ functions is
    // because CARB_PLUGIN_IMPL calls CARB_GLOBALS_EX which calls OMNI_GLOBALS_ADD_DEFAULT_CHANNEL and sets the channel
    // name to our plugin name: cesium.omniverse.plugin

    switch (_logLevel) {
        case omni::log::Level::eVerbose: {
            OMNI_LOG_VERBOSE("%s", formatMessage(msg));
        }
        case omni::log::Level::eInfo: {
            OMNI_LOG_INFO("%s", formatMessage(msg));
        }
        case omni::log::Level::eWarn: {
            OMNI_LOG_WARN("%s", formatMessage(msg));
        }
        case omni::log::Level::eError: {
            OMNI_LOG_ERROR("%s", formatMessage(msg));
        }
        case omni::log::Level::eFatal: {
            OMNI_LOG_FATAL("%s", formatMessage(msg));
        }
    }
}

void LoggerSink::flush_() {}

std::string LoggerSink::formatMessage(const spdlog::details::log_msg& msg) {
    // Frustratingly, spdlog::formatter isn't thread safe. So even though our sink
    // itself doesn't need to be protected by a mutex, the formatter does.
    // See https://github.com/gabime/spdlog/issues/897
    std::scoped_lock<std::mutex> lock(_formatMutex);

    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    return fmt::to_string(formatted);
}
} // namespace cesium::omniverse
