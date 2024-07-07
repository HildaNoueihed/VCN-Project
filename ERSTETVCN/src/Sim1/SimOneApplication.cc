//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "SimOneApplication.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "veins_inet/VeinsInetSampleMessage_m.h"

using namespace inet;

Define_Module(SimOneApplication);

SimOneApplication::SimOneApplication()
{
}

bool SimOneApplication::startApplication()
{

    if (getParentModule()->getIndex() == emergency_car) {

        auto callback = [this]() {
            EV_INFO << "Repeat callback executed\n";
             getParentModule()->getDisplayString().setTagArg("i", 1, "yellow");
             auto payload = makeShared<VeinsInetSampleMessage>();
             payload->setChunkLength(B(100));
             payload->setRoadId(traciVehicle->getRoadId().c_str());
             timestampPayload(payload);

             auto packet = createPacket("emergency");
             packet->insertAtBack(payload);
             sendPacket(std::move(packet));

            auto repeatCallback = [this]() {

            if (traciVehicle->getSpeed() != 0) {

                auto payload = makeShared<VeinsInetSampleMessage>();
                payload->setChunkLength(B(100));
                payload->setRoadId(traciVehicle->getRoadId().c_str());
                timestampPayload(payload);

                auto packet = createPacket("emergency");
                packet->insertAtBack(payload);
                sendPacket(std::move(packet));
            }

        }; timerManager.create(veins::TimerSpecification(repeatCallback).interval(SimTime(10, SIMTIME_S)));
        }; timerManager.create(veins::TimerSpecification(callback).oneshotAt(SimTime(28, SIMTIME_S)));

    }

    return true;
}

bool SimOneApplication::stopApplication()
{
    return true;
}

SimOneApplication::~SimOneApplication()
{
}

void SimOneApplication::processPacket(std::shared_ptr<inet::Packet> pk)
{
    if(getParentModule()->getIndex() != emergency_car){
        auto payload = pk->peekAtFront<VeinsInetSampleMessage>();

        EV_INFO << "Received packet: " << payload << endl;

        getParentModule()->getDisplayString().setTagArg("i", 1, "green");

        traciVehicle->changeLane(1,2);
        traciVehicle->setLaneChangeMode(0b0000001000);
        //changeRoute(payload->getRoadId(), 999.9);


        if (haveForwarded) return;

        auto packet = createPacket("relay");
        packet->insertAtBack(payload);
        sendPacket(std::move(packet));

        haveForwarded = true;
    }

}
