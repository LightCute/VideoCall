#include "socket.h"
#include <nlohmann/json.hpp>
#include <string>
#include "utilities/log.h"




WebSocket::WebSocket() {
    m_config = rtc::Configuration();
    m_ws = std::make_shared<rtc::WebSocket>();
    m_ws->onOpen([this]() {
        
    });

    m_ws->onClosed([this]() {
    });

    m_ws->onError([this](std::string err) {
    });

    m_ws->onMessage([this, wws = make_weak_ptr(m_ws)](auto data) {
		// data holds either std::string or rtc::binary
		if (!std::holds_alternative<std::string>(data))
			return;

		nlohmann::json message = nlohmann::json::parse(std::get<std::string>(data));

		auto it = message.find("id");
		if (it == message.end())
			return;

		auto id = it->get<std::string>();

		it = message.find("type");
		if (it == message.end())
			return;

		auto type = it->get<std::string>();

		std::shared_ptr<rtc::PeerConnection> pc;
		if (auto jt = m_peerConnectionMap.find(id); jt != m_peerConnectionMap.end()) {
			pc = jt->second;
		} else if (type == "offer") {
			std::cout << "Answering to " + id << std::endl;
			pc = createPeerConnection(m_config, wws, id);
		} else {
			return;
		}

		if (type == "offer" || type == "answer") {
			auto sdp = message["description"].get<std::string>();
			pc->setRemoteDescription(rtc::Description(sdp, type));
		} else if (type == "candidate") {
			auto sdp = message["candidate"].get<std::string>();
			auto mid = message["mid"].get<std::string>();
			pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
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
	    [](rtc::PeerConnection::State state) { std::cout << "State: " << state << std::endl; });

	pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
		std::cout << "Gathering State: " << state << std::endl;
	});

	pc->onLocalDescription([wws, id](rtc::Description description) {
		nlohmann::json message = {{"id", id},
		                {"type", description.typeString()},
		                {"description", std::string(description)}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onLocalCandidate([wws, id](rtc::Candidate candidate) {
		nlohmann::json message = {{"id", id},
		                {"type", "candidate"},
		                {"candidate", std::string(candidate)},
		                {"mid", candidate.mid()}};

		if (auto ws = wws.lock())
			ws->send(message.dump());
	});

	pc->onDataChannel([this, id](std::shared_ptr<rtc::DataChannel> dc) {
		std::cout << "DataChannel from " << id << " received with label \"" << dc->label() << "\""
		          << std::endl;

		dc->onOpen([this, wdc = make_weak_ptr(dc)]() {
			if (auto dc = wdc.lock())
				dc->send("Hello from " + m_localId);

		});

		dc->onClosed([this, id]() { std::cout << "DataChannel from " << id << " closed" << std::endl; });

		dc->onMessage([this, id](auto data) {
			// data holds either std::string or rtc::binary
            if (std::holds_alternative<std::string>(data))
            {            
                std::cout << "Message from " << id << " received: " << std::get<std::string>(data)
                        << std::endl;
                Log::info("[WebSocket] Message from [{}] received: {}", id, std::get<std::string>(data));
            }        
            else
            {   std::cout << "Binary message from " << id
                                << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
                Log::info("[WebSocket] Binary message from [{}] received, size={}", id, std::get<rtc::binary>(data).size());
            }   
		});

		m_dataChannelMap.emplace(id, dc);
        m_peer_id = id;
	});

	m_peerConnectionMap.emplace(id, pc);
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