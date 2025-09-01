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

#include "aircraft_manager.h"
#include "pilot_remote.pb.h"
#include "unifly.h"
#include "utilities.h"

namespace unifly
{
    AircraftManager::AircraftManager(UniFly* instance) :
        mEnv(instance)
    {
        ThisThreadIsXplane();
        XPLMRegisterFlightLoopCallback(&AircraftManager::AircraftMaintenanceCallback, -1.0f, this);
		XPMPRegisterPlaneNotifierFunc(&AircraftManager::AircraftNotifierCallback, this);
    }

    AircraftManager::~AircraftManager()
    {
        XPLMUnregisterFlightLoopCallback(&AircraftManager::AircraftMaintenanceCallback, nullptr);
		XPMPUnregisterPlaneNotifierFunc(&AircraftManager::AircraftNotifierCallback, nullptr);
    }

    void AircraftManager::HandleSpawn(const unifly::schema::RemoteSpawn& remote_spawn)
    {
        Log("handle spawn", remote_spawn.peer_id());
        // auto planeIter = mapPlanes.find()
    }

    void AircraftManager::HandleDespawn(const unifly::schema::RemoteDespawn& remote_despawn)
    {

    }

    void AircraftManager::HandleReportPosition(const unifly::schema::RemoteReportPosition& remote_report_position)
    {
    }

    void AircraftManager::HandleReportContext(const unifly::schema::RemoteReportContext& remote_report_context)
    {
        Log("handle report context %u", remote_report_context.peer_id());
    }

    float AircraftManager::AircraftMaintenanceCallback(float, float inElapsedTimeSinceLastFlightLoop, int, void* ref)
    {

    }

    void AircraftManager::AircraftNotifierCallback(XPMPPlaneID inPlaneID, XPMPPlaneNotification inNotification, void* ref)
    {

    }

}
