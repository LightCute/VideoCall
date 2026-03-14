#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include "websocket.h"
#include "../framework/event/event_bus.h"
#include "../business/connect/event/connect_success_event.h"
#include "../business/connect/event/connect_failed_event.h"
#include "../business/call/event/call_success_event.h"
#include "../business/call/event/call_failed_event.h"
#include "utilities/log.h"

void myCppLogCallback(rtc::LogLevel level, std::string message) {
    switch (level) {
    case rtc::LogLevel::Fatal:
    case rtc::LogLevel::Error:
        Log::error("[RTC] {}", message);
        break;
    case rtc::LogLevel::Warning:
        Log::warn("[RTC] {}", message);
        break;
    case rtc::LogLevel::Info:
        Log::info("[RTC] {}", message);
        break;
    case rtc::LogLevel::Debug:
        Log::debug("[RTC] {}", message);
        break;
    case rtc::LogLevel::Verbose:
        Log::trace("[RTC] {}", message);
        break;
    default:
        Log::info("[RTC] {}", message);
        break;
    }
}


WebSocket::WebSocket() {
    rtc::InitLogger(rtc::LogLevel::Debug, myCppLogCallback);
    localId = "abc1";
    rtc::Configuration config;
    ws = std::make_shared<rtc::WebSocket>();
    initCallbacks();
}

void WebSocket::initCallbacks() {

    ws->onOpen([this]() {
        EventBus::GetInstance().publish(
            std::make_unique<ConnectSuccessEvent>()
            );
    });

    ws->onClosed([this]() {
        // EventBus::GetInstance().publish(
        //     std::make_unique<ConnectFailedEvent>()
        //     );
    });

    ws->onError([this](std::string err) {
        EventBus::GetInstance().publish(
            std::make_unique<ConnectFailedEvent>()
            );
    });

    ws->onMessage([this, wws = make_weak_ptr(ws)](auto data) {
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
		if (auto jt = peerConnectionMap.find(id); jt != peerConnectionMap.end()) {
			pc = jt->second;
		} else if (type == "offer") {
			std::cout << "Answering to " + id << std::endl;
			pc = createPeerConnection(config, wws, id);
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

void WebSocket::connect(const std::string& url) {
    ws->open(url);
}

void WebSocket::send(const std::string& msg) {
    ws->send(msg);
}

void WebSocket::close() {
    ws->close();
}

void WebSocket::onMessage(MessageCallback cb) {
    messageCallback = cb;
}

void WebSocket::onOpen(OpenCallback cb) {
    openCallback = cb;
}

void WebSocket::onClose(CloseCallback cb) {
    closeCallback = cb;
}

void WebSocket::onError(ErrorCallback cb) {
    errorCallback = cb;
}

void WebSocket::connect2Peer(const std::string& peerId){
    auto pc = createPeerConnection(config, ws, peerId);
    auto dc = pc->createDataChannel("test");
    dc->onOpen([this,peerId, wdc = make_weak_ptr(dc)]() {
    std::cout << "DataChannel from " << peerId << " open" << std::endl;
        if (auto dc = wdc.lock())
            dc->send("Hello from " + localId);
            EventBus::GetInstance().publish(
                std::make_unique<CallSuccessEvent>()
                );
    });

    dc->onClosed([this,peerId]() { std::cout << "DataChannel from " << peerId << " closed" << std::endl; });

    dc->onMessage([this,peerId, wdc = make_weak_ptr(dc)](auto data) {
        // data holds either std::string or rtc::binary
        if (std::holds_alternative<std::string>(data))
        {            
            std::cout << "Message from " << peerId << " received: " << std::get<std::string>(data)
                    << std::endl;
            Log::info("[WebSocket] Message from [{}] received: {}", peerId, std::get<std::string>(data));
        }        
        else
        {   std::cout << "Binary message from " << peerId
                            << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
            Log::info("[WebSocket] Binary message from [{}] received, size={}", peerId, std::get<rtc::binary>(data).size());
        }    
    });

    dataChannelMap.emplace(peerId, dc);
    m_peer_id = peerId;
}


void WebSocket::send2Peer(const std::string& peerId, const std::string& msg) {
    auto it = dataChannelMap.find(m_peer_id);
    if (it != dataChannelMap.end()) {
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
				dc->send("Hello from " + localId);
            EventBus::GetInstance().publish(
            std::make_unique<CallSuccessEvent>()
            );

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

		dataChannelMap.emplace(id, dc);
        m_peer_id = id;
	});

	peerConnectionMap.emplace(id, pc);
	return pc;
};

