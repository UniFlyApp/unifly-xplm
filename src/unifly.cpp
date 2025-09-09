#include "unifly.h"
#include "aircraft_manager.h"
#include "data_ref_access.h"
#include "message.pb.h"
#include "pilot_local.pb.h"
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
        m_aircraftCount("unifly/num_aircraft", ReadOnly),

        m_beaconLights("sim/cockpit2/switches/beacon_on", ReadOnly),
        m_landingLights("sim/cockpit2/switches/landing_lights_on", ReadOnly),
        m_taxiLights("sim/cockpit2/switches/taxi_light_on", ReadOnly),
        m_navLights("sim/cockpit2/switches/navigation_lights_on", ReadOnly),
        m_strobeLights("sim/cockpit2/switches/strobe_lights_on", ReadOnly),

        m_latitude("sim/flightmodel/position/latitude", ReadOnly),
        m_longitude("sim/flightmodel/position/longitude", ReadOnly),
        m_pitch("sim/flightmodel/position/theta", ReadOnly),
        m_bank("sim/flightmodel/position/phi", ReadOnly),
        m_heading("sim/flightmodel/position/psi", ReadOnly),

        m_altitudeMslM("sim/flightmodel/position/elevation", ReadOnly),
        m_altitudeAglM("sim/flightmodel/position/y_agl", ReadOnly),
        m_altitudeStd("sim/flightmodel2/position/pressure_altitude", ReadOnly),

        m_altitudeTemperatureEffect("sim/weather/aircraft/altimeter_temperature_error", ReadOnly),
        m_barometerSeaLevel("sim/weather/barometer_sealevel_inhg", ReadOnly),

        m_groundSpeed("sim/flightmodel/position/groundspeed", ReadOnly),
        m_verticalSpeed("sim/flightmodel/position/vh_ind_fpm", ReadOnly),
        m_onGround("sim/flightmodel/failures/onground_any", ReadOnly),
        m_gearDown("sim/cockpit/switches/gear_handle_status", ReadOnly),
        m_flapRatio("sim/flightmodel/controls/flaprat", ReadOnly),
        m_speedbrakeRatio("sim/cockpit2/controls/speedbrake_ratio", ReadOnly)
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

        m_keepSocketAlive.store(true);
        m_socketThread = std::make_unique<std::thread>(&UniFly::SocketWorker, this);
    }

    void UniFly::Shutdown()
    {
        m_keepSocketAlive.store(false);

        if (m_socketThread) {
            Log("joining socket thread");
            m_socketThread->join();
            Log("joined socket thread");
        }
    }

    int CBIntPrefsFunc(const char*, [[maybe_unused]] const char* item, int defaultVal)
	{
        if (!strcmp(item, XPMP_CFG_ITM_HANDLE_DUP_ID))
            // If an XPMPPlaneId is not unique, throw an error rather than blindly choosing a new one
            return false;
		if (!strcmp(item, XPMP_CFG_ITM_CLAMPALL))
			return true;
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

	float UniFly::MainFlightLoop(float inElapsedSinceLastCall, float, int inCounter, void* ref)
	{
		auto* instance = static_cast<UniFly*>(ref);
		if (instance)
		{
			instance->InvokeQueuedCallbacks();
			instance->m_aiControlled = XPMPHasControlOfAIAircraft();
			instance->m_aircraftCount = XPMPCountPlanes();
			// UpdateMenuItems();

			// Send event frame
			unifly::schema::XPLMMessage event_frame_message;
            unifly::schema::EventFrame* event_frame = event_frame_message.mutable_event_frame();
            event_frame->set_counter(inCounter);
            instance->send_msg(event_frame_message);

            // Send local aircraft frequent
            unifly::schema::XPLMMessage read_frequent_message;
            unifly::schema::LocalReadFrequent* read_frequent = read_frequent_message.mutable_local_read_frequent();
            read_frequent->set_lat(instance->m_latitude);
            read_frequent->set_lon(instance->m_longitude);
            read_frequent->set_pitch(-1.0F * instance->m_pitch);
            read_frequent->set_bank(-1.0F * instance->m_bank);
            read_frequent->set_heading(instance->m_heading);

            read_frequent->set_ground_speed(instance->m_groundSpeed * 1.94384); //meters per second to knots
            read_frequent->set_vertical_speed(instance->m_verticalSpeed);
            read_frequent->set_on_ground(instance->m_onGround);

            double alt_msl = instance->m_altitudeMslM * 3.2808399; //meters to feet
            double alt_agl = instance->m_altitudeAglM * 3.2808399; //meters to feet
            double alt_std = instance->GetAltitudeStd();
            double alt_cal = alt_msl + instance->m_altitudeTemperatureEffect;
            // True elevation above mean sea level
            read_frequent->set_alt_msl(alt_msl);
            // True elevation above ground level
            read_frequent->set_alt_agl(alt_agl);
            // Std pressure altimeter (29.92)
            read_frequent->set_alt_std(alt_std);
            // Calibrated pressure altimter (local sea pressure)
            read_frequent->set_alt_cal(alt_cal);

            read_frequent->set_cg_height(0.0);
            instance->send_msg(read_frequent_message);

            // Send local aircraft infrequent
            // Counter counts in twos
            if (inCounter % (90 * 2) <= 1) {
                unifly::schema::XPLMMessage read_infrequent_message;
                unifly::schema::LocalReadInfrequent* read_infrequent = read_infrequent_message.mutable_local_read_infrequent();
                read_infrequent->set_lights_beacon(instance->m_beaconLights);
                read_infrequent->set_lights_landing(instance->m_landingLights);
                read_infrequent->set_lights_nav(instance->m_navLights);
                read_infrequent->set_lights_strobe(instance->m_strobeLights);
                read_infrequent->set_lights_taxi(instance->m_taxiLights);

                read_infrequent->set_flaps(instance->m_flapRatio);
                read_infrequent->set_spoilers(instance->m_speedbrakeRatio);
                read_infrequent->set_gear_down(instance->m_gearDown);

                instance->send_msg(read_infrequent_message);
            }
		}
		return -1.0;
	}

	void UniFly::SocketWorker()
	{
    	try {
            asio::io_context io;
            tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), XPLM_PORT));
            acceptor.non_blocking(true);
            Log("Waiting for connection...");

            while (m_keepSocketAlive.load())
            {
                auto socket = std::make_shared<tcp::socket>(io);
                asio::error_code ec;
                acceptor.accept(*socket, ec);

                if (ec == asio::error::would_block) {
                    // No connection yet, sleep briefly
                    // We don't want accept() blocking when m_keepSocketAlive has been set to false
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                } else if (ec) {
                    Log("Accept failed: &s", ec.message().c_str());
                    continue;
                }

                Log("Client connected\n");
                m_socket = socket;

                try {
                    // Send Open
                    QueueCallback([=] {
                        unifly::schema::XPLMMessage open_msg;
                        unifly::schema::EventOpen* open = open_msg.mutable_event_open();
                        open->set_version_xplm(XPLMVersion);
                        open->set_version_xplane(XPlaneVersion);
                        open->set_version_plugin(PLUGIN_VERSION);
                        open->set_models(XPMPModels);

                        send_msg(open_msg);
                    });

                    // Read
                    while (m_keepSocketAlive.load()) {
                        unifly::schema::XPlaneMessage message;
                        if (!recv_message(*socket, &message)) {
                            Log("Failed to receive message or client disconnected");
                            break;
                        }

                        ProcessPacket(message);
                        //TODO: Is there a buffer for us to free here? idk
                    }

                    m_socket = nullptr;

                    // Despawn planes
                    QueueCallback([=] {
                        DeleteAllAircraft();
                    });
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
				    m_aircraftManager->HandleDespawn(msg.remote_despawn().peer_id());
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
				Log("", err);
			}
		}
	}

	void UniFly::ReleaseTcasControl()
	{
		if (XPMPHasControlOfAIAircraft())
		{
			XPMPMultiplayerDisable();
			Log("xPilot has released TCAS control");
		}
	}

	void UniFly::DeleteAllAircraft()
	{

		m_aircraftManager->DespawnAll();
	}

    double UniFly::GetAltitudeStd()
    {
        if(IsXplane12()) {
            return m_altitudeStd;
        }

        float barometer = m_barometerSeaLevel * 33.8639; //29.92 -> 1013
        const double deltaPressure = (1013.25 - barometer) * 30.0; // 30ft per mbar
        return (m_altitudeMslM * 3.28084) + deltaPressure;
    }

	bool UniFly::IsXplane12()
	{
	    return XPlaneVersion >= 120000;
	}
}
