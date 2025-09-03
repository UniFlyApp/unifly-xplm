#pragma once
#include "data_ref_access.h"
#define PLUGIN_VERSION 100

#include "data_ref_owned.h"
#include "utilities.h"
#include "socket.h"

using asio::ip::tcp;

namespace unifly
{

    class AircraftManager;

    class UniFly {
    public:
        UniFly();
        ~UniFly();

		void Initialize();
		void Shutdown();
		void TryGetTcasControl();
		void ReleaseTcasControl();

		void AircraftDeleted(const std::string& callsign);
		void AircraftAdded(const std::string& callsign);
		void DeleteAllAircraft();


		/// All sending is allocated to occur on the xplane thread
		template<class T>
		void send_msg(const T& msg)
		{
		    if(m_socket) {
				if (!send_message(*m_socket, msg)) {
    				Log("failed to send a message");
    				m_keepSocketAlive = false;
    			}
			}
		}

	protected:

	    OwnedDataRef<int> m_aiControlled;
	    OwnedDataRef<int> m_aircraftCount;


        DataRefAccess<int> m_beaconLights;
        DataRefAccess<int> m_landingLights;
        DataRefAccess<int> m_taxiLights;
        DataRefAccess<int> m_navLights;
        DataRefAccess<int> m_strobeLights;
        DataRefAccess<double> m_latitude;
        DataRefAccess<double> m_longitude;
        DataRefAccess<double> m_altitudeMsl;
        DataRefAccess<double> m_pitch;
        DataRefAccess<double> m_heading;
        DataRefAccess<double> m_bank;
        DataRefAccess<double> m_altitudeAgl;
        DataRefAccess<double> m_altitudeStd;
        DataRefAccess<double> m_groundSpeed;
        DataRefAccess<double> m_verticalSpeed;
        DataRefAccess<int> m_onGround;
        DataRefAccess<int> m_gearDown;
        DataRefAccess<float> m_flapRatio;
        DataRefAccess<float> m_speedbrakeRatio;

		int XPlaneVersion, XPLMVersion, HostID, XPMPModels;

    private:
    	static float DeferredStartup(float, float, int, void* ref);
    	static float MainFlightLoop(float, float, int, void* ref);
    	bool InitializeXPMP();

        std::atomic<bool> m_keepSocketAlive = false;
        std::shared_ptr<tcp::socket> m_socket;
        std::unique_ptr<std::thread> m_socketThread;

		void SocketWorker();
		void ProcessPacket(const unifly::schema::XPlaneMessage msg);

		std::mutex m_mutex;
		std::deque<std::function<void()>> m_queuedCallbacks;
		void InvokeQueuedCallbacks();
		void QueueCallback(const std::function<void()>& cb);

		std::unique_ptr<AircraftManager> m_aircraftManager;
    };

};
