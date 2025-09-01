#include "unifly.h"
#include "aircraft_manager.h"
#include "data_ref_access.h"
#include "message.pb.h"
#include "utilities.h"
#include "socket.h"

#include "XPLMUtilities.h"
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
        m_aircraftManager = std::make_unique<AircraftManager>(this);

        XPLMGetVersions(&XPlaneVersion, &XPLMVersion, &HostID);
        XPLMRegisterFlightLoopCallback(DeferredStartup, -1.0f, this);
    }

    UniFly::~UniFly()
    {
        XPLMUnregisterFlightLoopCallback(DeferredStartup, this);
		XPLMUnregisterFlightLoopCallback(MainFlightLoop, this);
    }

    void UniFly::Initialize()
    {
        Log("init");
        InitializeXPMP();
        TryGetTcasControl();
        XPLMRegisterFlightLoopCallback(MainFlightLoop, -1.0f, this);

        XPMPModels = XPMPGetNumberOfInstalledModels();

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

        // TODO: check these
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

                Log("Waiting for connection...");
                acceptor.accept(socket);

                Log("Client connected\n");

                try {
                    unifly::schema::XPLMMessage open_msg;
                    unifly::schema::Open* open = open_msg.mutable_open();
                    open->set_version_xplm(XPLMVersion);
                    open->set_version_xplane(XPlaneVersion);
                    open->set_version_plugin(PLUGIN_VERSION);
                    open->set_models(XPMPModels);

                    if(!send_message(socket, open_msg)) {
                        Log("Failed to send Open message");
                        break;
                    }

                    while (true) {
                        unifly::schema::XPlaneMessage message;
                        if (!recv_message(socket, &message)) {
                            Log("Failed to receive message or client disconnected");
                            break;
                        }

                        ProcessPacket(message);
                        //TODO: Is there a buffer for us to free here? idk
                    }
                } catch (std::exception& e) {
                    Log("UniFly: Connection closed", e.what());
                }
            }
        } catch (std::exception& e) {
            Log("UniFly: Fatal error in socket", e.what());
        }
	}

	void UniFly::ProcessPacket(const unifly::schema::XPlaneMessage msg)
	{
	    switch (msg.kind_case()) {
			case unifly::schema::XPlaneMessage::kRemoteSpawn: {
				QueueCallback([msg = std::move(msg), this]() mutable
				{
				    m_aircraftManager->HandleSpawn(msg.remote_spawn());
				});

			    break;
			}
			case unifly::schema::XPlaneMessage::kRemoteDespawn: {
				QueueCallback([msg = std::move(msg), this]() mutable
				{
				    m_aircraftManager->HandleDespawn(msg.remote_despawn());
				});
			    break;
			}
			case unifly::schema::XPlaneMessage::kRemoteReportPosition: {
				QueueCallback([msg = std::move(msg), this]() mutable
				{
				    m_aircraftManager->HandleReportPosition(msg.remote_report_position());
				});
				break;
			}
			case unifly::schema::XPlaneMessage::kRemoteReportContext: {
				QueueCallback([msg = std::move(msg), this]() mutable
				{
				    m_aircraftManager->HandleReportContext(msg.remote_report_context());
				});
				break;
			}
			case unifly::schema::XPlaneMessage::KIND_NOT_SET: {
			    break;
			}
		}
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
