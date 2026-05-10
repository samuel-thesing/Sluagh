#include "sluagh/utils/Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

void Logger::init() {
	std::vector<spdlog::sink_ptr> sinks;

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
		"log/Log.log", true);
	file_sink->set_pattern("[%T] [%l]: %v");
	sinks.push_back(file_sink);

#ifndef NDEBUG
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_pattern("%^[%T] [%l]: %v%$");
	sinks.push_back(console_sink);
#endif

	logger =
		std::make_shared<spdlog::logger>("CORE", sinks.begin(), sinks.end());
	spdlog::register_logger(logger);

#ifndef NDEBUG
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
#else
	logger->set_level(spdlog::level::warn);
	logger->flush_on(spdlog::level::warn);
#endif
}
