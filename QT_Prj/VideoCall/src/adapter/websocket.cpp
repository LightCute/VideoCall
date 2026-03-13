#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include "websocket.h"
#include "../framework/event/event_bus.h"
#include "../business/connect/event/connect_success_event.h"
#include "../business/connect/event/connect_failed_event.h"
#include "utilities/log.h"

void WebSocket::myCppLogCallback(rtc::LogLevel level, std::string message) {
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
    //rtc::InitLogger(rtc::LogLevel::Verbose, this->myCppLogCallback);
    localId = "abcd";
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
		});

		dc->onClosed([this, id]() { std::cout << "DataChannel from " << id << " closed" << std::endl; });

		dc->onMessage([this, id](auto data) {
			// data holds either std::string or rtc::binary
			if (std::holds_alternative<std::string>(data))
				std::cout << "Message from " << id << " received: " << std::get<std::string>(data)
				          << std::endl;
			else
				std::cout << "Binary message from " << id
				          << " received, size=" << std::get<rtc::binary>(data).size() << std::endl;
		});

		dataChannelMap.emplace(id, dc);
	});

	peerConnectionMap.emplace(id, pc);
	return pc;
};

