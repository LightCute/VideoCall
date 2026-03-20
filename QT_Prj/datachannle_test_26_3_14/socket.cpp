#include "socket.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include "utilities/log.h"

// 辅助函数：将 PeerConnection::State 转为字符串
std::string peerConnectionStateToString(rtc::PeerConnection::State state) {
    switch (state) {
        case rtc::PeerConnection::State::New: return "New";
        case rtc::PeerConnection::State::Connecting: return "Connecting";
        case rtc::PeerConnection::State::Connected: return "Connected";
        case rtc::PeerConnection::State::Disconnected: return "Disconnected";
        case rtc::PeerConnection::State::Failed: return "Failed";
        case rtc::PeerConnection::State::Closed: return "Closed";
        default: return "Unknown State";
    }
}

// 辅助函数：将 PeerConnection::GatheringState 转为字符串
std::string gatheringStateToString(rtc::PeerConnection::GatheringState state) {
    switch (state) {
        case rtc::PeerConnection::GatheringState::New: return "New";
        case rtc::PeerConnection::GatheringState::InProgress: return "InProgress";
        case rtc::PeerConnection::GatheringState::Complete: return "Complete";
        default: return "Unknown GatheringState";
    }
}


WebSocket::WebSocket() {
    rtc::Configuration m_config;
    m_ws = std::make_shared<rtc::WebSocket>();
    m_ws->onOpen([this]() {
        
    });

    m_ws->onClosed([this]() {
    });

    m_ws->onError([this](std::string err) {
    });

    m_ws->onMessage([this,&m_config, wws = make_weak_ptr(m_ws)](auto data) {
		// data holds either std::string or rtc::binary
		if (!std::holds_alternative<std::string>(data))
			return;

        Log::info("[WebSocket] Message received: {}", std::get<std::string>(data));
		nlohmann::json message = nlohmann::json::parse(std::get<std::string>(data));

		auto it = message.find("id");
		if (it == message.end())
			return;

		auto id = it->get<std::string>();

		it = message.find("type");
		if (it == message.end())
			return;

		auto type = it->get<std::string>();

		if (auto jt = m_peerConnectionMap.find(id); jt != m_peerConnectionMap.end()) {
			m_pc = jt->second;
		} else if (type == "offer") {
			Log::info("[WebSocket] Answering to [{}]", id);
			m_pc = createPeerConnection(m_config, wws, id);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			auto sdp = message["description"].get<std::string>();
			m_pc->setRemoteDescription(rtc::Description(sdp, type));
		} else if (type == "candidate") {
			auto sdp = message["candidate"].get<std::string>();
			auto mid = message["mid"].get<std::string>();
			m_pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
		}
	});
}



void WebSocket::connect2Peer(const std::string& peerId){
    auto m_pc = createPeerConnection(m_config, m_ws, peerId);
    auto m_dc = m_pc->createDataChannel("test");
    m_dc->onOpen([this,peerId, wdc = make_weak_ptr(m_dc)]() {
        Log::info("[WebSocket] DataChannel from [{}] open", peerId);
        if (auto dc = wdc.lock())
            dc->send("Hello from " + m_localId);
    });

    m_dc->onClosed([this,peerId]() { 
        Log::info("[WebSocket] DataChannel from [{}] closed", peerId);
        });

    m_dc->onMessage([this,peerId, wdc = make_weak_ptr(m_dc)](auto data) {
        // data holds either std::string or rtc::binary
        if (std::holds_alternative<std::string>(data))
        {            
            Log::info("[WebSocket] Message from [{}] received: {}", peerId, std::get<std::string>(data));
        }        
        else
        {   
            Log::info("[WebSocket] Binary message from [{}] received, size={}", peerId, std::get<rtc::binary>(data).size());
        }    
    });

    m_dataChannelMap.emplace(peerId, m_dc);
    m_peer_id = peerId;
}


void WebSocket::send2Peer(const std::string& peerId, const std::string& msg) {
    auto it = m_dataChannelMap.find(m_peer_id);
    if (it != m_dataChannelMap.end()) {
        it->second->send(msg);
        Log::info("[WebSocket] Sent message to [{}]: {}", m_peer_id, msg);
    } else {
        Log::warn("[WebSocket] No data channel found for peer [{}]", m_peer_id);
    }
}


// Create and setup a PeerConnection
std::shared_ptr<rtc::PeerConnection> WebSocket::createPeerConnection(const rtc::Configuration &config,
                                                     std::weak_ptr<rtc::WebSocket> wws, std::string id) {
	auto pc = std::make_shared<rtc::PeerConnection>(config);

	pc->onStateChange(
	    [](rtc::PeerConnection::State state) { 
            
            Log::info("[PeerConnection] State changed: {}", peerConnectionStateToString(state));
        });

	pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
		Log::info("[PeerConnection] Gathering State changed: {}", gatheringStateToString(state));
	});

	pc->onLocalDescription([ wws, id](rtc::Description description) {
		nlohmann::json message = {{"id", id},
		                {"type", description.typeString()},
		                {"description", std::string(description)}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onLocalCandidate([ wws, id](rtc::Candidate candidate) {
		nlohmann::json message = {{"id", id},
		                {"type", "candidate"},
		                {"candidate", std::string(candidate)},
		                {"mid", candidate.mid()}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onDataChannel([this, id](std::shared_ptr<rtc::DataChannel> dc) {
        Log::info("[PeerConnection] DataChannel from [{}] received with label [{}]", id, dc->label());
		dc->onOpen([this, wdc = make_weak_ptr(dc)]() {
			if (auto dc = wdc.lock())
				dc->send("Hello from " + m_localId);

		});

		dc->onClosed([id]() { 
            Log::info("[PeerConnection] DataChannel from [{}] closed", id);
        });

		dc->onMessage([this, id](auto data) {
			// data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
            {            
                Log::info("[PeerConnection] Message from [{}] received: {}", id, std::get<std::string>(data));
            }        
            else
            {   
                Log::info("[PeerConnection] Binary message from [{}] received, size={}", id, std::get<rtc::binary>(data).size());
            }   
		});

		m_dataChannelMap.emplace(id, dc);
        Log::info("[PeerConnection] DataChannel from [{}] added to map", id);
	});

	m_peerConnectionMap.emplace(id, pc);
    Log::info("[PeerConnection] PeerConnection for [{}] created and added to map", id);
	return pc;
};

// 补全WebSocket类的connect/send/close函数实现
void WebSocket::connect2Server(const std::string& url, std::string& localId) {
    if (m_ws) {
        m_ws->open(url); // 调用rtc::WebSocket的open方法
        m_localId = localId;
    } else {
        Log::error("[WebSocket] WebSocket is empty {}", url);
    }
}

void WebSocket::send(const std::string& msg) {
    if (m_ws && m_ws->isOpen()) {
        m_ws->send(msg); // 调用rtc::WebSocket的send方法
    } else {
        Log::error("[WebSocket] WebSocket is not open {}", msg);
    }
}

void WebSocket::close() {
    if (m_ws) {
        m_ws->close(); // 调用rtc::WebSocket的close方法
    }
    // 清理资源
    m_peerConnectionMap.clear();
    m_dataChannelMap.clear();
    m_peer_id.clear();
}