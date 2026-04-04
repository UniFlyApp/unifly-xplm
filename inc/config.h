#include "settings.pb.h"
#include "unifly.h"

namespace unifly {

class Config {
    public:
  		static Config& GetInstance();
		~Config() = default;
		Config(const Config&) = delete;
		void operator=(const Config&) = delete;
		Config(Config&&)noexcept = default;
		Config& operator=(Config&&)noexcept = default;

		void LoadConfig(const unifly::schema::v1::Settings& settings);
		void SaveConfig(UniFly* instance);

		void SetAircraftLabelShow(bool status) { m_aircraftLabelShow = status; }
		bool GetAircraftLabelShow() const { return m_aircraftLabelShow; }

		void SetAircraftLabelType(schema::v1::AircraftLabelType type) { m_aircraftLabelType = type; }
		schema::v1::AircraftLabelType GetAircraftLabelType() const { return m_aircraftLabelType; }

		void SetAircraftLabelColour(schema::v1::AircraftLabelColour colour) {m_aircraftLabelColour = colour; }
		schema::v1::AircraftLabelColour GetAircraftLabelColour() const { return m_aircraftLabelColour; }

		void SetAircraftLabelRange(schema::v1::AircraftLabelRange distance) { m_aircraftLabelRange = distance; }
		schema::v1::AircraftLabelRange GetAircraftLabelRange() const { return m_aircraftLabelRange; }

		void SetAircraftLabelVisibilityCutoff(bool value) { m_aircraftLabelVisibilityCutoff = value; }
		bool GetAircraftLabelVisibilityCutoff() const { return m_aircraftLabelVisibilityCutoff; }

	private:
		Config() = default;

		bool m_aircraftLabelShow = true;
		bool m_aircraftLabelVisibilityCutoff = true;
		schema::v1::AircraftLabelType m_aircraftLabelType = schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT_AIRLINE;
		schema::v1::AircraftLabelColour m_aircraftLabelColour = schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_YELLOW;
		schema::v1::AircraftLabelRange m_aircraftLabelRange = schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_5;
};

const char* AircraftLabelTypeToString(schema::v1::AircraftLabelType type);
const char* AircraftLabelRangeToString(schema::v1::AircraftLabelRange type);
const char* AircraftLabelColourToString(schema::v1::AircraftLabelColour type);

schema::v1::AircraftLabelType AircraftLabelTypeNext(schema::v1::AircraftLabelType type);
schema::v1::AircraftLabelRange AircraftLabelRangeNext(schema::v1::AircraftLabelRange range);
schema::v1::AircraftLabelColour AircraftLabelColourNext(schema::v1::AircraftLabelColour colour);

float AircraftLabelRangeValue(schema::v1::AircraftLabelRange range);

}
