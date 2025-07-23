#pragma once

#include "data_ref_owned.h"

namespace unifly
{

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

    private:
    	static float DeferredStartup(float, float, int, void* ref);
    	static float MainFlightLoop(float, float, int, void* ref);
    	bool InitializeXPMP();

        bool m_keepSocketAlive = false;
		nng_socket m_socket;
		std::unique_ptr<std::thread> m_socketThread;

		void SocketWorker();

		std::mutex m_mutex;
		std::deque<std::function<void()>> m_queuedCallbacks;
		void InvokeQueuedCallbacks();
		void QueueCallback(const std::function<void()>& cb);
    };

};
