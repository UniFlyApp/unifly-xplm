#include "unifly.h"
#include "data_ref_access.h"
#include "utilities.h"
#include "socket.h"

#include "XPMPMultiplayer.h"

using asio::ip::tcp;

// Defined seperately in the Rust code
#define XPLM_PORT 9925

namespace unifly
{
    UniFly::UniFly() :
        m_aiControlled("unifly/ai_controlled", ReadOnly),
        m_aircraftCount("unifly/num_aircraft", ReadOnly)
    {
        XPLMRegisterFlightLoopCallback(DeferredStartup, -1.0f, this);
    }

    UniFly::~UniFly()
    {
        XPLMUnregisterFlightLoopCallback(DeferredStartup, this);
		XPLMUnregisterFlightLoopCallback(MainFlightLoop, this);
    }

    void UniFly::Initialize()
    {
        InitializeXPMP();
        TryGetTcasControl();
        XPLMRegisterFlightLoopCallback(MainFlightLoop, -1.0f, this);

        m_keepSocketAlive = true;
        m_socketThread = std::make_unique<std::thread>(&UniFly::SocketWorker, this);
    }

    void UniFly::Shutdown()
    {
        //TODO: Shutdown message on the socket?

        m_keepSocketAlive = false;

        if (m_socketThread) {
            m_socketThread->join();
        }
    }

    int CBIntPrefsFunc(const char*, [[maybe_unused]] const char* item, int defaultVal)
	{
		if (!strcmp(item, XPMP_CFG_ITM_CLAMPALL))
			return 0;
		return defaultVal;
	}

	bool UniFly::InitializeXPMP()
	{
    	const std::string pathResources(GetPluginPath() + "Resources");

    	auto err = XPMPMultiplayerInit("UniFly", pathResources.c_str(), &CBIntPrefsFunc);

    	if (*err)
    	{
    		Log("UniFly: Error initializing multiplayer: %s", err);
    		XPMPMultiplayerCleanup();
    		return false;
    	}

        // Load our CSL models
        auto res = XPMPLoadCSLPackage(pathResources.c_str());
        if (*err) {
            Log("UniFly: Error while loading CSL packages: %s", res);
            return false;
        }

    	// XPMPEnableAircraftLabels(Config::GetInstance().GetShowHideLabels());
    	// XPMPSetAircraftLabelDist(Config::GetInstance().GetMaxLabelDistance(), Config::GetInstance().GetLabelCutoffVis());
    	// XPMPSetAudioDevice(Config::GetInstance().GetAudioDevice());

    	return true;
	}

	float UniFly::DeferredStartup(float, float, int, void* ref)
	{
		auto* instance = static_cast<UniFly*>(ref);
		if (instance)
			instance->Initialize();
		return 0;
	}


	float UniFly::MainFlightLoop(float inElapsedSinceLastCall, float, int, void* ref)
	{
		auto* instance = static_cast<UniFly*>(ref);
		if (instance)
		{
			instance->InvokeQueuedCallbacks();
			instance->m_aiControlled = XPMPHasControlOfAIAircraft();
			instance->m_aircraftCount = XPMPCountPlanes();
			// UpdateMenuItems();
		}
		return -1.0;
	}

	void UniFly::SocketWorker()
	{
    	try {
            asio::io_context io;
            tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), XPLM_PORT));

            while (m_keepSocketAlive)
                {
                tcp::socket socket(io);

                Log("[server] Waiting for connection...\n");
                acceptor.accept(socket); // blocking accept

                Log("[server] Client connected\n");

                try {
                    while (true) {
                        unifly::schema::XPlaneMessage message;
                        if (!recv_message(socket, &message)) {
                            // If recv_message fails, it's likely due to a disconnected client
                            std::cerr << "Failed to receive message or client disconnected.\n";
                            break;
                        }

                        std::cout << "Received message:\n" << message.DebugString() << std::endl;

                        unifly::schema::XPLMMessage res;
                        unifly::schema::ReadLocalFrequent* freq = res.mutable_read_local_frequent();
                        freq->set_lat(255.0);

                        std::cout << "Return message:\n" << res.DebugString() << std::endl;

                        if (!send_message(socket, res)) {
                            std::cerr << "Failed to send message or client disconnected.\n";
                            break;
                        }
                    }

                    for (;;) {
                        char data[1024];
                        std::size_t len = socket.read_some(asio::buffer(data));
                        std::cout << "[server] Received: "
                                << std::string(data, len) << "\n";
                    }
                } catch (std::exception& e) {
                    std::cout << "[server] Connection closed: " << e.what() << "\n";
                }
            }
        } catch (std::exception& e) {
            std::cerr << "[fatal] " << e.what() << "\n";
        }

		// while (m_keepSocketAlive)
		// {
		// 	char* buffer;
		// 	size_t bufferLen;

		// 	int err;
		// 	err = nng_recv(m_socket, &buffer, &bufferLen, NNG_FLAG_ALLOC);

		// 	if (err == 0)
		// 	{
			    // Log("UniFly received message of %s bytes", bufferLen);
		// 		// BaseDto dto;
		// 		// auto obj = msgpack::unpack(reinterpret_cast<const char*>(buffer), bufferLen);

		// 		// try
		// 		// {
		// 		// 	obj.get().convert(dto);
		// 		// 	ProcessPacket(dto);
		// 		// }
		// 		// catch (const msgpack::type_error& e) {}

		// 		// nng_free(buffer, bufferLen);
		// 	}
		// }
	}

	void UniFly::QueueCallback(const std::function<void()>& cb)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		m_queuedCallbacks.push_back(cb);
	}

	void UniFly::InvokeQueuedCallbacks()
	{
		std::deque<std::function<void()>> temp;
		{
			std::lock_guard<std::mutex> lck(m_mutex);
			std::swap(temp, m_queuedCallbacks);
		}
		while (!temp.empty())
		{
			auto cb = temp.front();
			temp.pop_front();
			cb();
		}
	}

	void callbackRequestTcasAgain(void*)
	{
		XPMPMultiplayerEnable(callbackRequestTcasAgain);
	}

	void UniFly::TryGetTcasControl()
	{
		if (!XPMPHasControlOfAIAircraft())
		{
			auto err = XPMPMultiplayerEnable(callbackRequestTcasAgain);
			if (*err)
			{
				Log("UniFly:", err);
			}
		}
	}

	void UniFly::ReleaseTcasControl()
	{
		if (XPMPHasControlOfAIAircraft())
		{
			XPMPMultiplayerDisable();
			Log("UniFly: xPilot has released TCAS control");
		}
	}

	void UniFly::DeleteAllAircraft()
	{
	    // TODO
		// m_aircraftManager->RemoveAllPlanes();
	}
}
