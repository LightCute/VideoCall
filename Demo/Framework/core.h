#pragma once
#include <thread>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <exception>
#include "abstract_command.h"
#include "blocking_queue.h"
#include "log.h"

class Core : public AbstractCommandDispatcher {
public:
    // 构造函数：启动后台消费线程
    Core() : m_running(true) {
        m_worker_thread = std::thread(&Core::run, this);
        Log::info("Core: Background thread started, id: {}",
            Log::threadIdToString(m_worker_thread.get_id()));
    }

    // 析构函数：优雅停止后台线程
    ~Core() {
        Log::info("Core: Destroying, stopping background thread");
        m_running = false;
        m_queue.stop(); // 新增：先停止队列，让waitAndPop()退出
        if (m_worker_thread.joinable()) {
            m_worker_thread.join(); // 等待线程退出
        }
        std::cout << "Core: Background thread exited." << std::endl;
    }

    // 提交命令（对外接口，和main.cpp匹配）
    void postCommand(std::unique_ptr<AbstractCommand> cmd) override {
        if (!cmd) {
            Log::error("Core: Attempt to post null command");
            throw std::invalid_argument("Core: cmd is null");
        }
        Log::debug("Core: Posting command type: {}", cmd->getCommandType());
        m_queue.push(std::move(cmd));
    }

private:
    // 后台线程的消费循环
    void run() {
        Log::debug("Core: Command processing loop started");
        while (m_running) {
            auto cmd = m_queue.waitAndPop(); // 阻塞获取命令
            if (!cmd || !m_running) {
                break;
            }
            try {
                Log::trace("Core: Executing command type: {}", 
                    cmd->getCommandType());
                bool execResult = cmd->execute();
                // 可选：根据返回值判断执行结果
                if (!execResult) {
                    Log::error("Core: Command[{}] execute returned false", 
                        cmd->getCommandType());
                }else {
                    Log::debug("Core: Command[{}] executed successfully", 
                        cmd->getCommandType());
                }
            } catch (const std::exception& e) {
                Log::error("Core: Command[{}] execute failed: {}", 
                    cmd->getCommandType(), e.what());
            } catch (...) {
                Log::error("Core: Command[{}] execute failed with unknown exception", 
                    cmd->getCommandType());
            }
        }
        Log::debug("Core: Command processing loop exited");
    }

    BlockingQueue<std::unique_ptr<AbstractCommand>> m_queue;          // 命令队列
    std::thread m_worker_thread;   // 后台消费线程
    std::atomic<bool> m_running;   // 线程退出标志（原子变量保证线程安全）
};