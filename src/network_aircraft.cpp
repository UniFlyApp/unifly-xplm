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

#include "network_aircraft.h"

namespace unifly
{
    NetworkAircraft::NetworkAircraft(
        const int peer_id,
        const AircraftVisualState& _visualState,
        const std::string& _callsign,
        const std::string& _icaoType,
		const std::string& _icaoAirline,
		const std::string& _livery,
		XPMPPlaneID _modeS_id = 0,
		const std::string& _modelName = ""
    ) : XPMP2::Aircraft(_icaoType, _icaoAirline, _livery, _modeS_id, _modelName)
    {
        visual_state = _visualState;
    }


    void NetworkAircraft::UpdatePosition(float _frameRatePeriod, int)
    {
        SetLocation(
            visual_state.lat,
            visual_state.lon,
            visual_state.alt_msl
        );
        SetPitch(visual_state.pitch);
        SetRoll(visual_state.bank);
        SetHeading(visual_state.heading);
        SetOnGrnd(true);
    }

}
