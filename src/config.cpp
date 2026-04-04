#include "config.h"
#include "message.pb.h"
#include "plugin.h"
#include "settings.pb.h"
#include "utilities.h"

namespace unifly {

    Config& Config::GetInstance()
	{
		static auto&& config = Config();
		return config;
	}

	void Config::LoadConfig(const unifly::schema::v1::Settings& settings)
	{
	    LOG_MSG("load config");

	    SetAircraftLabelShow(settings.aircraft_label_show());
		SetAircraftLabelColour(settings.aircraft_label_colour());
		SetAircraftLabelRange(settings.aircraft_label_range());
		SetAircraftLabelType(settings.aircraft_label_type());
		SetAircraftLabelVisibilityCutoff(settings.aircraft_label_visibility_cutoff());

		UpdateMenuItems();
	}

	void Config::SaveConfig(UniFly* instance)
	{
        LOG_MSG("save config");
	    unifly::schema::v1::XPLMMessage settings_message;
		unifly::schema::v1::Settings* settings = settings_message.mutable_settings();

		settings->set_aircraft_label_show(GetAircraftLabelShow());
		settings->set_aircraft_label_colour(GetAircraftLabelColour());
		settings->set_aircraft_label_range(GetAircraftLabelRange());
		settings->set_aircraft_label_type(GetAircraftLabelType());
		settings->set_aircraft_label_visibility_cutoff(GetAircraftLabelVisibilityCutoff());

		instance->send_msg(settings_message);

		UpdateMenuItems();
	}

	const char* AircraftLabelTypeToString(schema::v1::AircraftLabelType type) {
        switch (type) {
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRCRAFT: return "AC";
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRLINE: return "AL";
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN: return "CS";
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT: return "CS / AC";
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT_AIRLINE: return "CS / AC / AL";
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_UNSPECIFIED:
            default:
                return "";
        }
    }

    schema::v1::AircraftLabelType AircraftLabelTypeNext(schema::v1::AircraftLabelType type) {
        switch (type) {
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRCRAFT: return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRLINE;
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRLINE: return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN;
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN: return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT;
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT: return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT_AIRLINE;
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_CALLSIGN_AIRCRAFT_AIRLINE: return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRCRAFT;
            case schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_UNSPECIFIED:
            default:
                return schema::v1::AircraftLabelType::AIRCRAFT_LABEL_TYPE_AIRCRAFT;
        }
    }

    const char* AircraftLabelRangeToString(schema::v1::AircraftLabelRange range) {
        switch (range) {
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_1:  return "1nm";
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_3:  return "3nm";
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_5:  return "5nm";
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_10: return "10nm";
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_20: return "20nm";
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_UNSPECIFIED:
            default:
                return "unspecified";
        }
    }

    schema::v1::AircraftLabelRange AircraftLabelRangeNext(schema::v1::AircraftLabelRange range) {
        switch (range) {
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_1:  return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_3;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_3:  return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_5;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_5:  return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_10;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_10: return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_20;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_20: return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_1;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_UNSPECIFIED:
            default:
                return schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_1;
        }
    }

    float AircraftLabelRangeValue(schema::v1::AircraftLabelRange range) {
        switch (range) {
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_1:  return 1.0;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_3:  return 3.0;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_5:  return 5.0;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_10: return 10.0;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_MILES_20: return 20.0;
            case schema::v1::AircraftLabelRange::AIRCRAFT_LABEL_RANGE_UNSPECIFIED:
            default:
                return 20.0;
        }
    }

    const char* AircraftLabelColourToString(schema::v1::AircraftLabelColour colour) {
        switch (colour) {
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_YELLOW: return "yellow";
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_RED: return "red";
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_GREEN: return "green";
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_BLUE: return "blue";
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_UNSPECIFIED:
            default:
                return "unspecified";
        }
    }

    schema::v1::AircraftLabelColour AircraftLabelColourNext(schema::v1::AircraftLabelColour colour) {
        switch (colour) {
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_YELLOW: return schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_RED;
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_RED: return schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_GREEN;
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_GREEN: return schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_BLUE;
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_BLUE: return schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_YELLOW;
            case schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_UNSPECIFIED:
            default:
                return schema::v1::AircraftLabelColour::AIRCRAFT_LABEL_COLOUR_YELLOW;
        }
    }





}
