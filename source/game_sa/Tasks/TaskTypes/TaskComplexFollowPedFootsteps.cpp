#include "StdInc.h"

#include "TaskComplexFollowPedFootsteps.h"
// #include "PointRoute.h"

void CTaskComplexFollowPedFootsteps::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexFollowPedFootsteps, 0x870CC0, 12);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x694E20);
    RH_ScopedVMTInstall(CreateNextSubTask, 0x694EE0);
    RH_ScopedVMTInstall(CreateFirstSubTask, 0x695000);
}

// 0x694E20
CTaskComplexFollowPedFootsteps::CTaskComplexFollowPedFootsteps(CPed* ped) : CTaskComplex() {
    m_targetPed               = ped;
    m_updateGoToPoint         = false;
    m_subTaskCreateCheckTimer = CTimer::GetTimeInMS();
    m_lineOfSightCheckTimer   = 0;
    m_pointRoute              = nullptr;
    m_moveState               = PEDMOVE_WALK;

    CEntity::SafeRegisterRef(m_targetPed);

    m_pointRoute = new CPointRoute();
}

CTaskComplexFollowPedFootsteps* CTaskComplexFollowPedFootsteps::Constructor(CPed* ped) {
    this->CTaskComplexFollowPedFootsteps::CTaskComplexFollowPedFootsteps(ped);
    return this;
}

CTaskComplexFollowPedFootsteps::~CTaskComplexFollowPedFootsteps() {
    CEntity::SafeCleanUpRef(m_targetPed);

    delete m_pointRoute;
    m_pointRoute = nullptr;
}

bool CTaskComplexFollowPedFootsteps::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return plugin::CallMethodAndReturn<bool, 0x694ED0, CTaskComplexFollowPedFootsteps*, CPed*, eAbortPriority, const CEvent*>(this, ped, priority, event);
}

// NOTSA: Inlined at 0x694F6E and 0x695019
bool CTaskComplexFollowPedFootsteps::IsFarFromTargetPed(const CPed* ped) const {
    // The squared distance is compared with 1.4 directly, not 1.4 squared
    return (m_targetPed->GetPosition() - ped->GetPosition()).SquaredMagnitude() > 1.4f;
}

// 0x694EE0
CTask* CTaskComplexFollowPedFootsteps::CreateNextSubTask(CPed* ped) {
    if (!m_targetPed) {
        return CreateSubTask(TASK_FINISHED, ped);
    }

    switch (m_pSubTask->GetTaskType()) {
    case TASK_SIMPLE_HIT_HEAD:
        return CreateSubTask(TASK_SIMPLE_STAND_STILL, ped);
    case TASK_SIMPLE_STAND_STILL:
    case TASK_SIMPLE_GO_TO_POINT:
        return CreateSubTask(m_pointRoute && !m_pointRoute->IsEmpty() ? TASK_SIMPLE_GO_TO_POINT : TASK_SIMPLE_STAND_STILL, ped);
    case TASK_COMPLEX_SEEK_ENTITY:
        return CreateSubTask(IsFarFromTargetPed(ped) ? TASK_COMPLEX_SEEK_ENTITY : TASK_SIMPLE_STAND_STILL, ped);
    default:
        return CreateSubTask(TASK_FINISHED, ped);
    }
}

// 0x695000
CTask* CTaskComplexFollowPedFootsteps::CreateFirstSubTask(CPed* ped) {
    if (!m_targetPed) {
        return CreateSubTask(TASK_FINISHED, ped);
    }
    return CreateSubTask(IsFarFromTargetPed(ped) ? TASK_COMPLEX_SEEK_ENTITY : TASK_SIMPLE_STAND_STILL, ped);
}

CTask* CTaskComplexFollowPedFootsteps::ControlSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x695090, CTaskComplexFollowPedFootsteps*, CPed*>(this, ped);
}

CTask* CTaskComplexFollowPedFootsteps::CreateSubTask(eTaskType taskType, CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x695E40, CTaskComplexFollowPedFootsteps*, eTaskType, CPed*>(this, taskType, ped);
}
