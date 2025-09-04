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

#include "XPMPMultiplayer.h"
#include "network_aircraft.h"
#include "pilot_remote.pb.h"
#include "unifly.h"
#include <memory>

namespace unifly
{
    typedef std::map<XPMPPlaneID, std::unique_ptr<NetworkAircraft>> mapPlanesTy;
    extern mapPlanesTy mapPlanes;

    class AircraftManager
    {
    public:
        AircraftManager(UniFly* instance);
        virtual ~AircraftManager();

        void HandleSpawn(const unifly::schema::RemoteSpawn& remote_spawn);
        void HandleDespawn(const int peer_id);
        void HandleReportPosition(const unifly::schema::RemoteReportPosition& remote_report_position);
        void HandleReportContext(const unifly::schema::RemoteReportContext& remote_report_context);

    private:
        NetworkAircraft* GetAircraft(const int peer_id);

        static float AircraftMaintenanceCallback(float, float, int, void* ref);
        static void AircraftNotifierCallback(XPMPPlaneID inPlaneID, XPMPPlaneNotification inNotification, void* ref);

        UniFly* mEnv;

        std::thread::id m_xplaneThread;
        void ThisThreadIsXplane()
        {
            m_xplaneThread = std::this_thread::get_id();
        }
        bool IsXplaneThread()const
        {
            return std::this_thread::get_id() == m_xplaneThread;
        }


    };
}
