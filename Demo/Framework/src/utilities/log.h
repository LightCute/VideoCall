#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>


class Log {
public:
    // 新增：日志模式枚举
    enum class Mode {
        Sync,   // 同步模式
        Async   // 异步模式
    };

    // 统一初始化接口
    // filename: 日志文件名
    // mode: 同步或异步，默认异步
    // level: 日志输出级别，默认 info (即 info/warn/error/critical 会输出，trace/debug 会被过滤)
    static void init(const std::string& filename = "app.log", 
                     Mode mode = Mode::Async,
                     spdlog::level::level_enum level = spdlog::level::info) {
        static bool initialized = false;
        if (initialized) return;

        try {
            // 1. 创建 sinks (控制台 + 文件)
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
            
            // 注意：sink 本身的级别设为最低，主要靠全局 logger 级别来过滤
            console_sink->set_level(spdlog::level::trace);
            file_sink->set_level(spdlog::level::trace);

            std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };

            // 2. 根据模式创建 logger
            std::shared_ptr<spdlog::logger> logger;
            
            if (mode == Mode::Async) {
                // 异步模式：初始化线程池
                spdlog::init_thread_pool(8192, 1);
                logger = std::make_shared<spdlog::async_logger>(
                    "async_logger", 
                    sinks.begin(), 
                    sinks.end(),
                    spdlog::thread_pool(), 
                    spdlog::async_overflow_policy::block
                );
            } else {
                // 同步模式
                logger = std::make_shared<spdlog::logger>("sync_logger", sinks.begin(), sinks.end());
            }

            // 3. 设置默认 logger
            spdlog::set_default_logger(logger);
            
            // 4. 设置格式
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");

            // 5. 设置全局过滤级别 (核心改动在这里)
            spdlog::set_level(level); 

            initialized = true;
            std::cout << "Log init success. Mode: " << (mode == Mode::Async ? "Async" : "Sync") 
                      << ", Level: " << spdlog::level::to_string_view(level).data() << std::endl;
        } catch (const spdlog::spdlog_ex &ex) {
            std::cerr << "Log init failed: " << ex.what() << std::endl;
        }
    }

    // 辅助函数：将 thread::id 转为 string
    static std::string threadIdToString(const std::thread::id& id) {
        std::ostringstream oss;
        oss << id;
        return oss.str();
    }    

    // 日志接口
    template<typename... Args>
    static void trace(const char* fmt, Args&&... args) { spdlog::trace(fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    static void debug(const char* fmt, Args&&... args) { spdlog::debug(fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    static void info(const char* fmt, Args&&... args) { spdlog::info(fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    static void warn(const char* fmt, Args&&... args) { spdlog::warn(fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    static void error(const char* fmt, Args&&... args) { spdlog::error(fmt, std::forward<Args>(args)...); }

    template<typename... Args>
    static void critical(const char* fmt, Args&&... args) { spdlog::critical(fmt, std::forward<Args>(args)...); }
};