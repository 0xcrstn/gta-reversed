#include "StdInc.h"

#include "TaskAllocatorKillThreatsBasic.h"
#include <InterestingEvents.h>
#include <TaskComplexKillPedGroupOnFoot.h>
#include <TaskComplexSequence.h>
#include <TaskSimpleLookAbout.h>

// 0x69C710
CTaskAllocatorKillThreatsBasic::CTaskAllocatorKillThreatsBasic(CPed* threat) :
    m_Threat{threat}
{
}

// 0x69D170
void CTaskAllocatorKillThreatsBasic::AllocateTasks(CPedGroupIntelligence* intel) {
    intel->FlushTasks(intel->GetPedTaskPairs(), nullptr);
    intel->FlushTasks(intel->GetSecondaryPedTaskPairs(), nullptr);

    if (!m_Threat) {
        return;
    }

    auto* const group = &intel->GetPedGroup();
    if (auto* const threatsGroup = m_Threat->GetGroup()) {
        if (threatsGroup == group) {
            NOTSA_LOG_DEBUG("ComputeKillThreatsBasicResponse() - threat ped already in group"); // vanilla
        } else {
            CPed* closest[TOTAL_PED_GROUP_MEMBERS]{};
            ComputeClosestPeds(*group, *threatsGroup, closest);
            for (int32 i = 0; i < TOTAL_PED_GROUP_MEMBERS; i++) {
                auto* mem = group->GetMembership().GetMember(i);
                if (!mem || mem->IsPlayer()) {
                    continue;
                }
                intel->SetEventResponseTask(
                    mem,
                    CTaskComplexKillPedGroupOnFoot{ CPedGroups::GetGroupId(threatsGroup), closest[i] }
                );
            }
            g_InterestingEvents.Add(CInterestingEvents::GANG_FIGHT, group->GetMembership().GetLeader()); // 0x69D436
        }
    } else { // 0x69D2EB
        for (auto* const mem : group->GetMembership().GetMembers()) {
            if (mem->IsPlayer()) {
                continue;
            }
            intel->SetEventResponseTask(
                mem,
                CTaskComplexSequence{
                    new CTaskComplexKillPedOnFoot{ m_Threat },
                    new CTaskSimpleLookAbout{ CGeneral::GetRandomNumberInRange(1'000u, 2'000u) },
                }
            );
        }
        g_InterestingEvents.Add(CInterestingEvents::GANG_ATTACKING_PED, group->GetMembership().GetLeader()); // 0x69D436
    }
}

// 0x69C7E0
CTaskAllocator* CTaskAllocatorKillThreatsBasic::ProcessGroup(CPedGroupIntelligence* intel) {
    m_Timer.StartIfNotAlready(0);
    if (m_Timer.IsOutOfTime()) {
        m_Timer.Start(5'000);
        AllocateTasks(intel);
    }
    return this;
}

// 0x69C850
void CTaskAllocatorKillThreatsBasic::ComputeClosestPeds(CPedGroup& group1, CPedGroup& group2, CPed** peds) {
    auto& mships1 = group1.GetMembership();
    auto& mships2 = group2.GetMembership();

    rng::fill_n(peds, TOTAL_PED_GROUP_MEMBERS, nullptr);

    // Squared distances between the alive, non-player members of `group1` and the alive members of `group2`
    std::array<std::array<float, TOTAL_PED_GROUP_MEMBERS>, TOTAL_PED_GROUP_MEMBERS> distsSq;
    for (auto& row : distsSq) {
        rng::fill(row, FLT_MAX);
    }
    for (auto i = 0; i < TOTAL_PED_GROUP_MEMBERS; i++) {
        auto* const mem1 = mships1.GetMember(i);
        if (!mem1 || !mem1->IsAlive() || mem1->IsPlayer()) {
            continue;
        }
        for (auto j = 0; j < TOTAL_PED_GROUP_MEMBERS; j++) {
            auto* const mem2 = mships2.GetMember(j);
            if (!mem2 || !mem2->IsAlive()) {
                continue;
            }
            distsSq[i][j] = CVector::DistSqr(mem1->GetPosition(), mem2->GetPosition());
        }
    }

    // Pair up the closest members, then the closest of the remaining ones, and so on
    for (auto n = 0; n < TOTAL_PED_GROUP_MEMBERS; n++) {
        float closestDistSq = FLT_MAX;
        int32 closestMem1   = -1;
        int32 closestMem2   = -1;
        for (auto i = 0; i < TOTAL_PED_GROUP_MEMBERS; i++) {
            for (auto j = 0; j < TOTAL_PED_GROUP_MEMBERS; j++) {
                if (distsSq[i][j] < closestDistSq) { // Strict, so the entries set to `FLT_MAX` are never picked
                    closestDistSq = distsSq[i][j];
                    closestMem1   = i;
                    closestMem2   = j;
                }
            }
        }
        if (closestMem1 < 0) {
            continue;
        }
        for (auto k = 0; k < TOTAL_PED_GROUP_MEMBERS; k++) {
            distsSq[closestMem1][k] = FLT_MAX;
            distsSq[k][closestMem2] = FLT_MAX;
        }
        peds[closestMem1] = mships2.GetMember(closestMem2);
    }

    // The rest of `group1`'s members get `group2`'s leader, or if it's dead, its first alive follower
    auto* filler = mships2.GetLeader();
    if (!filler || !filler->IsAlive()) {
        filler = nullptr;
        for (auto* const follower : mships2.GetFollowers()) {
            if (follower->IsAlive()) {
                filler = follower;
                break;
            }
        }
        if (!filler) {
            return;
        }
    }
    for (auto i = 0; i < TOTAL_PED_GROUP_MEMBERS; i++) {
        if (mships1.GetMember(i) && !peds[i]) {
            peds[i] = filler;
        }
    }
}

void CTaskAllocatorKillThreatsBasic::InjectHooks() {
    RH_ScopedVirtualClass(CTaskAllocatorKillThreatsBasic, 0x870e90, 6);
    RH_ScopedCategory("Tasks/Allocators");

    RH_ScopedInstall(Constructor, 0x69C710);
    RH_ScopedInstall(Destructor, 0x69C780);

    RH_ScopedGlobalInstall(ComputeClosestPeds, 0x69C850);
    RH_ScopedVMTInstall(GetType, 0x69C770);
    RH_ScopedVMTInstall(AllocateTasks, 0x69D170);
    RH_ScopedVMTInstall(ProcessGroup, 0x69C7E0);
}
