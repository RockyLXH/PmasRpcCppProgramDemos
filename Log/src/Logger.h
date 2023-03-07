#pragma once

#include "spdlog/spdlog.h"

#ifdef LOG_LIB
	#define LOG_API __declspec(dllexport)
#else
	#define LOG_API __declspec(dllimport)
#endif

class LOG_API Logger
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
	static std::shared_ptr<spdlog::logger> s_Logger;

};

#define LOG_TRACE(...)		Logger::GetLogger()->trace(__VA_ARGS__);
#define LOG_DEBUG(...)		Logger::GetLogger()->debug(__VA_ARGS__);
#define LOG_INFO(...)		Logger::GetLogger()->info(__VA_ARGS__);
#define LOG_WARN(...)		Logger::GetLogger()->warn(__VA_ARGS__);
#define LOG_ERROR(...)		Logger::GetLogger()->error(__VA_ARGS__);
#define LOG_CRITICAL(...)	Logger::GetLogger()->critical(__VA_ARGS__);
