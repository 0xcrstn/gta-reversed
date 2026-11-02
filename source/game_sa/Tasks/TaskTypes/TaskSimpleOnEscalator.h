#pragma once

#include "TaskSimpleStandStill.h"

class CPed;

class NOTSA_EXPORT_VTABLE CTaskSimpleOnEscalator : public CTaskSimpleStandStill {
public:
    static constexpr auto Type = TASK_SIMPLE_ON_ESCALATOR;

    static void InjectHooks();

    // Inlined in `Clone` and `CEventHandler::ComputeOnEscalatorResponse` (0x4BC150)
    CTaskSimpleOnEscalator() :
        CTaskSimpleStandStill{ 0, true, false, 8.0f } {}

    // 0x4B8750
    ~CTaskSimpleOnEscalator() override = default;

    // 0x4B8720
    eTaskType GetTaskType() const override { return Type; }

    // 0x4B86C0
    CTask* Clone() const override { return new CTaskSimpleOnEscalator{}; }

    bool ProcessPed(CPed* ped) override;
};

VALIDATE_SIZE(CTaskSimpleOnEscalator, 0x20);
