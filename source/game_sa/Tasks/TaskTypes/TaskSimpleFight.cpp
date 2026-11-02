#include "StdInc.h"

#include "TaskSimpleFight.h"

// 0x61C470
CTaskSimpleFight::CTaskSimpleFight(CEntity* entity, int32 nCommand, uint32 nIdlePeriod) : CTaskSimple() {
    plugin::CallMethod<0x61C470, CTaskSimpleFight*, CEntity*, int32, uint32>(this, entity, nCommand, nIdlePeriod);
}

// 0x61C530
CTaskSimpleFight::~CTaskSimpleFight() {
    plugin::CallMethod<0x61C530, CTaskSimpleFight*>(this);
}

// 0x61C5E0
bool CTaskSimpleFight::ControlFight(CEntity* entity, uint8 command) {
    m_bIsInControl = true;
    if (m_pTargetEntity != entity) {
        CEntity::ChangeEntityReference(m_pTargetEntity, entity);
    }
    if ((int8)command > (int8)m_nNextCommand) { // NOTE: Signed comparison
        m_nNextCommand = command;
    }
    return true;
}

// 0x4ABDC0
bool CTaskSimpleFight::IsComboSet() {
    const auto& combo = m_aComboData[std::max(0, m_nComboSet - MELEE_COMBO_UNARMED_1)];
    return combo.m_wFlags & (0x10u << (m_nCurrentMove & 31)); // The shift count is masked like x86's `shl` does
}

// 0x4ABDF0
bool CTaskSimpleFight::IsHitComboSet() {
    if (m_nCurrentMove > FIGHT_ATTACK_HIT_3) {
        return false;
    }
    const auto& combo = m_aComboData[std::max(0, m_nComboSet - MELEE_COMBO_UNARMED_1)];
    return combo.m_wFlags & (0x1000u << (m_nCurrentMove & 31)); // The shift count is masked like x86's `shl` does
}

// 0x4ABDA0
AssocGroupId CTaskSimpleFight::GetComboAnimGroupID() {
    return m_aComboData[std::max(0, m_nComboSet - MELEE_COMBO_UNARMED_1)].m_nAnimGroup;
}

// 0x5BEDC0
void CTaskSimpleFight::LoadMeleeData() {
    ZoneScoped;

    plugin::Call<0x5BEDC0>();
}

// 0x6239F0
bool CTaskSimpleFight::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return plugin::CallMethodAndReturn<bool, 0x6239F0, CTaskSimpleFight*, CPed*, eAbortPriority, const CEvent*>(this, ped, priority, event);
}

// 0x629920
bool CTaskSimpleFight::ProcessPed(CPed* ped) {
    return plugin::CallMethodAndReturn<bool, 0x629920, CTaskSimpleFight*, CPed*>(this, ped);
}

void CTaskSimpleFight::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleFight, 0x86D684, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(ControlFight, 0x61C5E0);
    RH_ScopedInstall(IsComboSet, 0x4ABDC0);
    RH_ScopedInstall(IsHitComboSet, 0x4ABDF0);
    RH_ScopedInstall(GetComboAnimGroupID, 0x4ABDA0);
    RH_ScopedInstall(LoadMeleeData, 0x5BEDC0, { .Reversed = false });
}

CTaskSimpleFight* CTaskSimpleFight::Constructor(CEntity* entity, int32 nCommand, uint32 nIdlePeriod) {
    this->CTaskSimpleFight::CTaskSimpleFight(entity, nCommand, nIdlePeriod);
    return this;
}

CTaskSimpleFight* CTaskSimpleFight::Destructor() {
    this->CTaskSimpleFight::~CTaskSimpleFight();
    return this;
}
