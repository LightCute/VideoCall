// CallManager.cpp
#include "CallManager.h"
#include <ctime> // 用于生成简单的sessionId

void CallManager::initCall(const std::string& peerName, const std::string& peerIp, int peerPort, bool isCaller) {
    // 步骤1：生成唯一sessionId（阶段1可简化，后续可优化为UUID）
    std::string sessionId = "call_" + std::to_string(time(nullptr));

    // 步骤2：写入CallSession（真相源，核心操作）
    currentSession = CallSession(
        sessionId,
        peerName,
        peerIp,
        peerPort,
        isCaller,
        OldCallState::CONNECTING
        );

    // 步骤3：同步旧字段（兼容现有逻辑，100%安全）
    peer_ = peerName;
    peerIp_ = peerIp;
    peerPort_ = peerPort;
    isCaller_ = isCaller;

    // 步骤4：原有逻辑完全保留（FSM/UI无感知）
    oldFsmState = OldCallState::CONNECTING;
    notifyUiOutStateChanged(oldFsmState);
}

// 示例：修改“更新对端IP”方法（其他写入方法同理）
void CallManager::updatePeerIp(const std::string& newIp) {
    // 先写入Session（真相源）
    if (currentSession.has_value()) { // 有会话时才更新
        currentSession->peerIp = newIp;
    }
    // 同步旧字段（兼容）
    peerIp_ = newIp;
}

void CallManager::endCurrentSession() {
    // 步骤1：清理Session（真相源）
    if (currentSession.has_value()) {
        currentSession.reset(); // 销毁会话，释放资源
    }

    // 步骤2：重置旧状态/字段（兼容现有逻辑）
    oldFsmState = OldCallState::IDLE;
    peer_ = "";
    peerIp_ = "";
    peerPort_ = 0;
    isCaller_ = false;

    // 步骤3：统一释放所有通话资源（原有分散的释放逻辑迁移至此）
    releaseMediaResources();   // 释放媒体资源
    closeNetworkConnection();  // 关闭网络连接
    clearCallLogs();           // 清理通话日志

    // 步骤4：原有UI通知逻辑不变（无感知）
    notifyUiOutStateChanged(oldFsmState);
}

void CallManager::onHangup() {
    // 原有分散逻辑删除 → 统一调用
    endCurrentSession();
}

// 2. 网络异常断开
void CallManager::onNetworkError() {
    endCurrentSession();
}

// 3. 媒体流结束
void CallManager::onMediaStreamEnd() {
    endCurrentSession();
}

// 4. 对端挂断
void CallManager::onPeerHangup() {
    endCurrentSession();
}

// 5. 通话超时
void CallManager::onTimeout() {
    endCurrentSession();
}
