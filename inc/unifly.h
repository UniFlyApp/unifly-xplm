#pragma once
#define PLUGIN_VERSION 100

#include "data_ref_owned.h"

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

	protected:

	    OwnedDataRef<int> m_aiControlled;
	    OwnedDataRef<int> m_aircraftCount;

		int XPlaneVersion, XPLMVersion, HostID, XPMPModels;

    private:
    	static float DeferredStartup(float, float, int, void* ref);
    	static float MainFlightLoop(float, float, int, void* ref);
    	bool InitializeXPMP();

        bool m_keepSocketAlive = false;
		// nng_socket m_socket;
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
