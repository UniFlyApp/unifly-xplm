#pragma once

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
    	static float DeferredStartup(float, float, int, void* ref);
    	static float MainFlightLoop(float, float, int, void* ref);
    	bool InitializeXPMP();

     private:
    };

};
