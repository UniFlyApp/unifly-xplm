#pragma once
#include "data_ref_access.h"
// Change the expected unifly plugin version in net_shared.rs
#define UNIFLY_PLUGIN_VERSION 110

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

		double GetAltitudeStd();
		bool IsXplane12();

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
        DataRefAccess<float> m_pitch;
        DataRefAccess<float> m_heading;
        DataRefAccess<float> m_bank;

        /// The elevation above MSL of the aircraft
        DataRefAccess<double> m_altitudeMslM;
        DataRefAccess<float> m_altitudeAglM;
        /// User airplane altitude as pressure altitude in standard atmosphere
        DataRefAccess<double> m_altitudeStd;

        DataRefAccess<float> m_altitudeTemperatureEffect;
        DataRefAccess<float> m_barometerSeaLevel;

        DataRefAccess<float> m_groundSpeed;
        DataRefAccess<float> m_verticalSpeed;
        DataRefAccess<int> m_onGround;
        DataRefAccess<int> m_gearDown;
        DataRefAccess<float> m_flapRatio;
        DataRefAccess<float> m_speedbrakeRatio;

		int XPlaneVersion, XPLMVersion, HostID;

    private:
    	static float DeferredStartup(float, float, int, void* ref);
    	static float MainFlightLoop(float, float, int, void* ref);
    	bool InitializeXPMP();

        std::atomic<bool> m_keepSocketAlive = false;
        std::shared_ptr<tcp::socket> m_socket;
        std::unique_ptr<std::thread> m_socketThread;

		void SocketWorker();
		void ProcessPacket(const unifly::schema::v1::XPlaneMessage msg);

		std::mutex m_mutex;
		std::deque<std::function<void()>> m_queuedCallbacks;
		void InvokeQueuedCallbacks();
		void QueueCallback(const std::function<void()>& cb);

		std::unique_ptr<AircraftManager> m_aircraftManager;
    };

};
