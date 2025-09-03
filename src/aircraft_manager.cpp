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
#include "XPMPAircraft.h"
#include "XPMPMultiplayer.h"
#include "network_aircraft.h"
#include "pilot_remote.pb.h"
#include "unifly.h"
#include "utilities.h"

namespace unifly
{
    mapPlanesTy mapPlanes;

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
        const int peer_id = remote_spawn.peer_id();
        Log("handle spawn %u", peer_id);

        auto planeIt = mapPlanes.find(peer_id);
        if (planeIt != mapPlanes.end()) {
            // Remove the plane
            HandleDespawn(peer_id);
            return;
        }

        const std::string& callsign = "JBU257";
        const std::string& airline = "JBU";
        const std::string& aircraft = "A32N";

        AircraftVisualState visual_state{};
        visual_state.lat = remote_spawn.lat();
        visual_state.lon = remote_spawn.lon();
        visual_state.pitch = remote_spawn.pitch();
        visual_state.bank = remote_spawn.bank();
        visual_state.heading = remote_spawn.heading();
        // visual_state.alt_msl = remote_spawn.alt_msl();
        // visual_state.alt_agl = remote_spawn.alt_agl();

        NetworkAircraft* plane = new NetworkAircraft(peer_id, visual_state, callsign, aircraft, airline, "", 0, "");
        mapPlanes.emplace(peer_id, std::move(plane));
    }

    void AircraftManager::HandleDespawn(const int peer_id)
    {
    }

    void AircraftManager::HandleReportPosition(const unifly::schema::RemoteReportPosition& remote_report_position)
    {
        const int peer_id = remote_report_position.peer_id();
        auto aircraft = GetAircraft(peer_id);
        if (!aircraft) return;

        aircraft->visual_state.lat = remote_report_position.lat();
        aircraft->visual_state.lon = remote_report_position.lon();
        aircraft->visual_state.pitch = remote_report_position.pitch();
        aircraft->visual_state.bank = remote_report_position.bank();
        aircraft->visual_state.heading = remote_report_position.heading();
    }

    void AircraftManager::HandleReportContext(const unifly::schema::RemoteReportContext& remote_report_context)
    {
        const int peer_id = remote_report_context.peer_id();
        auto aircraft = GetAircraft(peer_id);
        if (!aircraft) return;
    }

    NetworkAircraft* AircraftManager::GetAircraft(const int peer_id)
    {
        auto planeIt = mapPlanes.find(peer_id);
        if (planeIt == mapPlanes.end()) return nullptr;
        return planeIt->second.get();
    }

    float AircraftManager::AircraftMaintenanceCallback(float, float inElapsedTimeSinceLastFlightLoop, int, void* ref)
    {

    }

    void AircraftManager::AircraftNotifierCallback(XPMPPlaneID inPlaneID, XPMPPlaneNotification inNotification, void* ref)
    {
        auto* instance = static_cast<AircraftManager*>(ref);
        if (instance)
        {
            XPMP2::Aircraft* pAc = XPMP2::AcFindByID(inPlaneID);
            if (pAc)
            {
                unifly::schema::XPLMMessage message;
                if (inNotification == xpmp_PlaneNotification_Created)
                {
                    unifly::schema::RemoteSpawned* remote_spawned = message.mutable_remote_spawned();

                    // pAc->
                    // remote_spawned->set_peer_id(peer)

                }
                else if (inNotification == xpmp_PlaneNotification_Destroyed)
                {
                    unifly::schema::RemoteDespawned* remote_spawned = message.mutable_remote_despawned();

                }
                else if (inNotification == xpmp_PlaneNotification_ModelChanged)
                {
                    // unifly::schema::RemoteDespawned* remote_spawned = message.mutable_remote_despawned();
                    //TODO!
                }

                instance->mEnv->send_msg(message);
            }
        }
    }

}
