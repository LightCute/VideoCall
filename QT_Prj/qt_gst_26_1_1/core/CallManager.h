// CallManager.h - 逻辑管理类，持有CallSession
#pragma once
#include <optional>
#include "CallSession.h" // 引入独立的CallSession类

class CallManager {
private:
    // 核心新增：用optional管理会话（安全处理无会话场景）
    std::optional<CallSession> currentSession;

    // 保留旧状态/字段（FSM/UI仍使用，阶段1不删除）
    OldCallState oldFsmState = OldCallState::IDLE;
    std::string peer_;
    std::string peerIp_;
    int peerPort_ = 0;
    bool isCaller_ = false;

public:
    // 原有核心方法（完全保留，不做任何修改）
    void fsmHandleState(OldCallState newState);
    void notifyUiOutStateChanged(OldCallState state);

    // 后续步骤要实现的方法（先声明）
    void initCall(const std::string& peerName, const std::string& peerIp, int peerPort, bool isCaller);
    void updatePeerIp(const std::string& newIp);
    void endCurrentSession();

    // 原有结束相关方法（保留，后续替换实现）
    void onHangup();
    void onNetworkError();
    void onMediaStreamEnd();
    void onPeerHangup();
    void onTimeout();

private:
    // 原有资源释放方法（保留，后续统一调用）
    void releaseMediaResources();
    void closeNetworkConnection();
    void clearCallLogs();
};
