/*
 * xPilot: X-Plane pilot client for VATSIM
 * Copyright (C) 2019-2024 Justin Shannon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
*/

#pragma once

#include "XPMPAircraft.h"
#include "terrain_probe.h"
#define INCLUDE_FMOD_SOUND 1

namespace unifly
{
    struct AircraftVisualState
    {
        double lat;
        double lon;
        double pitch;
        double bank;
        double heading;
        double alt_msl;
    };

	inline float RpmToDegree(float rpm, double s)
	{
		return rpm / 60.0f * float(s) * 360.0f;
	}

    class NetworkAircraft : public XPMP2::Aircraft
    {

    public:
        NetworkAircraft(
            const int _peer_id,
            const AircraftVisualState& _visualState,
            const std::string& _callsign,
            const std::string& _icaoType,
      		const std::string& _icaoAirline,
      		const std::string& _livery,
    		const std::string& _modelName
        );

        const int peer_id;

        TerrainProbe terrain_probe;
        double elevation_terrain;

        // XPMPPlaneSurfaces_t surfaces;
        AircraftVisualState visual_state;
        bool engines;

    protected:
        virtual void UpdatePosition(float, int) override;

    };

}
