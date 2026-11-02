#include "StdInc.h"
#include "MentalHealth.h"

#include "Events/EventHealthLow.h"
#include "Events/EventHealthReallyLow.h"
#include "Events/EventLowAngerAtPlayer.h"
#include "Events/EventHighAngerAtPlayer.h"

void CMentalState::InjectHooks() {
    RH_ScopedClass(CMentalState);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Process, 0x6008A0);
}

// 0x6008A0
void CMentalState::Process(CPed& ped) {
    // Health values are stored truncated to a byte (`_ftol` result, low byte only)
    m_pedHealth = (uint8)(int32)ped.m_fHealth;
    if (ped.IsInVehicle()) {
        m_vehicleHealth = (uint8)(int32)ped.m_pVehicle->m_fHealth;
    }

    if (!ped.bInVehicle) {
        if (m_oldPedHealth >= 50) {
            if (m_pedHealth < 10) {
                ped.GetEventGroup().Add(CEventHealthReallyLow{});
            } else if (m_pedHealth < 50) {
                ped.GetEventGroup().Add(CEventHealthLow{});
            }
        } else if (m_oldPedHealth >= 10 && m_pedHealth < 10) {
            ped.GetEventGroup().Add(CEventHealthReallyLow{});
        }
    } else {
        // Original quirk: vehicle health (0..1000) is compared against 600/300 after
        // being truncated to a byte, so the old value is never >= 300 and no event is ever raised here.
        const int32 vehHealth = m_vehicleHealth, oldVehHealth = m_oldVehicleHealth;
        if (oldVehHealth >= 600) {
            if (vehHealth < 300) {
                ped.GetEventGroup().Add(CEventHealthReallyLow{});
            } else if (vehHealth < 600) {
                ped.GetEventGroup().Add(CEventHealthLow{});
            }
        } else if (oldVehHealth >= 300 && vehHealth < 300) {
            ped.GetEventGroup().Add(CEventHealthReallyLow{});
        }
    }

    if (m_LastAngerAtPlayer <= 3) {
        if (m_AngerAtPlayer > 6) {
            ped.GetEventGroup().Add(CEventHighAngerAtPlayer{});
        } else if (m_AngerAtPlayer > 3) {
            ped.GetEventGroup().Add(CEventLowAngerAtPlayer{});
        }
    } else if (m_LastAngerAtPlayer <= 6 && m_AngerAtPlayer > 6) {
        ped.GetEventGroup().Add(CEventHighAngerAtPlayer{});
    }

    m_LastAngerAtPlayer = m_AngerAtPlayer;
    m_oldPedHealth      = m_pedHealth;
    m_oldVehicleHealth  = m_vehicleHealth;
}
