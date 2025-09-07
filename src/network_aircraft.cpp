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
#include "utilities.h"

namespace unifly
{
    NetworkAircraft::NetworkAircraft(
        const int peer_id,
        const AircraftVisualState& _visualState,
        const std::string& _callsign,
        const std::string& _icaoType,
		const std::string& _icaoAirline,
		const std::string& _livery,
		const std::string& _modelName = ""
    ) : XPMP2::Aircraft(_icaoType, _icaoAirline, _livery, peer_id, _modelName),
        peer_id(peer_id)
    {
        strScpy(acInfoTexts.tailNum, _callsign.c_str(), sizeof(acInfoTexts.tailNum));
		strScpy(acInfoTexts.icaoAcType, acIcaoType.c_str(), sizeof(acInfoTexts.icaoAcType));
		strScpy(acInfoTexts.icaoAirline, acIcaoAirline.c_str(), sizeof(acInfoTexts.icaoAirline));

		SetVisible(true);
		SetLocation(_visualState.lat, _visualState.lon, _visualState.alt_msl);
		SetHeading(_visualState.heading);
		SetPitch(_visualState.pitch);
		SetRoll(_visualState.bank);

		label = "NetworkAircraft";
        colLabel[0] = 0.0f;             // green
        colLabel[1] = 1.0f;
        colLabel[2] = 0.0f;

		visual_state = _visualState;

		SetOnGrnd(false);
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
        // SetOnGrnd(true);

        if (engines) {
            SetEngineRotRpm(1200);
            SetPropRotRpm(GetEngineRotRpm());
            SetEngineRotAngle(GetEngineRotAngle() + RpmToDegree(GetEngineRotRpm(), _frameRatePeriod));
			while (GetEngineRotAngle() >= 360.0f)
			{
				SetEngineRotAngle(GetEngineRotAngle() - 360.0f);
			}
			SetPropRotAngle(GetEngineRotAngle());
			SetThrustRatio(1.0f);
        }
        else
		{
			SetEngineRotRpm(0.0f);
			SetPropRotRpm(0.0f);
			SetEngineRotAngle(0.0f);
			SetPropRotAngle(0.0f);
			SetThrustRatio(0.0f);
		}
    }

}
