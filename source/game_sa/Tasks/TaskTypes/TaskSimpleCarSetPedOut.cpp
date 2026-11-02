#include "StdInc.h"

#include "TaskSimpleCarSetPedOut.h"

void CTaskSimpleCarSetPedOut::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleCarSetPedOut, 0x86EEB8, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(PositionPedOutOfCollision, 0x6479B0, { .Reversed = false });

    RH_ScopedVMTInstall(Clone, 0x649F50);
    RH_ScopedVMTInstall(ProcessPed, 0x647D10, { .Reversed = false });
}

CTaskSimpleCarSetPedOut::CTaskSimpleCarSetPedOut(CVehicle* targetVehicle, eTargetDoor nTargetDoor, bool bSwitchOffEngine, bool warpingOutOfCar) :
    m_nTargetDoor{ nTargetDoor },
    m_pTargetVehicle{ targetVehicle },
    m_bSwitchOffEngine{ bSwitchOffEngine },
    m_bWarpingOutOfCar{ warpingOutOfCar }
{
    CEntity::SafeRegisterRef(m_pTargetVehicle);
}

CTaskSimpleCarSetPedOut::~CTaskSimpleCarSetPedOut() {
    CEntity::SafeCleanUpRef(m_pTargetVehicle);
}

// 0x6479B0
void CTaskSimpleCarSetPedOut::PositionPedOutOfCollision(CPed* ped, CVehicle* veh, int32 door) {
    return plugin::Call<0x6479B0>(ped, veh, door);
}

// 0x649F50
CTask* CTaskSimpleCarSetPedOut::Clone() const {
    auto* const task             = new CTaskSimpleCarSetPedOut{ m_pTargetVehicle, m_nTargetDoor, m_bSwitchOffEngine };
    task->m_bWarpingOutOfCar     = m_bWarpingOutOfCar;
    task->m_bFallingOutOfCar     = m_bFallingOutOfCar;
    task->m_bKnockedOffBike      = m_bKnockedOffBike;
    task->m_nDoorFlagsToClear    = m_nDoorFlagsToClear;
    task->m_nNumGettingInToClear = m_nNumGettingInToClear;
    return task;
}

// 0x647D10
bool CTaskSimpleCarSetPedOut::ProcessPed(CPed* ped) {
    return plugin::CallMethodAndReturn<bool, 0x647D10, CTask*, CPed*>(this, ped);
}
