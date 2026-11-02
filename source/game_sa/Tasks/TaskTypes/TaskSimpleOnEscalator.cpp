#include "StdInc.h"

#include "TaskSimpleOnEscalator.h"

void CTaskSimpleOnEscalator::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleOnEscalator, 0x85B8DC, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedVMTInstall(Clone, 0x4B86C0);
    RH_ScopedVMTInstall(GetTaskType, 0x4B8720);
    RH_ScopedVMTInstall(ProcessPed, 0x62F520);
}

// 0x62F520
bool CTaskSimpleOnEscalator::ProcessPed(CPed* ped) {
    CTaskSimpleStandStill::ProcessPed(ped); // The result is ignored

    auto* const step = ped->m_pContactEntity;
    if (!step) {
        return true;
    }
    if (step->m_nModelIndex != ModelIndices::MI_ESCALATORSTEP && step->m_nModelIndex != ModelIndices::MI_ESCALATORSTEP8) {
        return true;
    }

    // Face the direction the step moves in
    const auto& stepMoveSpeed = step->AsPhysical()->GetMoveSpeed();
    ped->m_fAimingRotation    = CGeneral::LimitRadianAngle(CGeneral::GetRadianAngleBetweenPoints(stepMoveSpeed.x, stepMoveSpeed.y, 0.0f, 0.0f));
    return false;
}
