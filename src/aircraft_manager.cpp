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

        auto planeIt = GetAircraft(peer_id);
        if (planeIt) {
            // Remove the plane
            HandleDespawn(peer_id);
            return;
        }

        const std::string& callsign = "EZY257";
        const std::string& airline = "EZY";
        const std::string& aircraft = "A320";

        AircraftVisualState visual_state{};
        visual_state.lat = remote_spawn.lat();
        visual_state.lon = remote_spawn.lon();
        visual_state.pitch = remote_spawn.pitch();
        visual_state.bank = remote_spawn.bank();
        visual_state.heading = remote_spawn.heading();
        visual_state.alt_msl = remote_spawn.alt_msl();
        // visual_state.alt_agl = remote_spawn.alt_agl();


        NetworkAircraft* plane = new NetworkAircraft(peer_id, visual_state, callsign, aircraft, airline, "", "");
        mapPlanes.emplace(peer_id, std::move(plane));
    }

    void AircraftManager::HandleDespawn(const int peer_id)
    {
        // auto aircraft = GetAircraft(peer_id);
        // if(!aircraft) return;
        mapPlanes.erase(peer_id);
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

        aircraft->SetFlapRatio(remote_report_context.flaps());
        aircraft->SetSlatRatio(remote_report_context.flaps());
        aircraft->SetSpoilerRatio(remote_report_context.spoilers() ? 1.0f : 0.0f);
        aircraft->SetSpeedbrakeRatio(remote_report_context.spoilers() ? 1.0f : 0.0f);
        aircraft->SetGearRatio(remote_report_context.gear() ? 1.0f : 0.0f); //TODO: Add || on ground once I verify that this doesn't need ! or summat
        aircraft->SetLightsStrobe(remote_report_context.lights_strobe());
        aircraft->SetLightsTaxi(remote_report_context.lights_taxi());
        aircraft->SetLightsNav(remote_report_context.lights_nav());
        aircraft->SetLightsLanding(remote_report_context.lights_landing());
        aircraft->SetLightsBeacon(remote_report_context.lights_beacon());
        aircraft->engines = remote_report_context.engines();
    }

    NetworkAircraft* AircraftManager::GetAircraft(const int peer_id)
    {
        auto planeIt = mapPlanes.find(peer_id);
        if (planeIt == mapPlanes.end()) return nullptr;
        return planeIt->second.get();
    }

    float AircraftManager::AircraftMaintenanceCallback(float, float inElapsedTimeSinceLastFlightLoop, int, void* ref)
    {
        return -1.0f;
    }

    void AircraftManager::AircraftNotifierCallback(XPMPPlaneID peer_id, XPMPPlaneNotification inNotification, void* ref)
    {
        auto* instance = static_cast<AircraftManager*>(ref);
        if (instance)
        {
            XPMP2::Aircraft* pAc = XPMPGetAircraft(peer_id);
            if (pAc)
            {

                Log("Plane (%u) of type %s, airline %s, model %s, label '%s' %s",
                       peer_id,
                       pAc->acIcaoType.c_str(),
                       pAc->acIcaoAirline.c_str(),
                       pAc->GetModelName().c_str(),
                       pAc->label.c_str(),
                       inNotification == xpmp_PlaneNotification_Created ? "created" :
                       inNotification == xpmp_PlaneNotification_ModelChanged ? "changed" : "destroyed");

                unifly::schema::XPLMMessage message;
                if (inNotification == xpmp_PlaneNotification_Created)
                {
                    unifly::schema::RemoteSpawned* remote_spawned = message.mutable_remote_spawned();
                    remote_spawned->set_peer_id(peer_id);

                }
                else if (inNotification == xpmp_PlaneNotification_Destroyed)
                {
                    unifly::schema::RemoteDespawned* remote_despawned = message.mutable_remote_despawned();
                    remote_despawned->set_peer_id(peer_id);

                }
                else if (inNotification == xpmp_PlaneNotification_ModelChanged)
                {
                    // unifly::schema::RemoteDespawned* remote_spawned = message.mutable_remote_despawned();
                    //TODO!
                }

                instance->mEnv->send_msg(message);
            } else {
                Log("notifier callback did not find the plane id");
            }
        }
    }

}
