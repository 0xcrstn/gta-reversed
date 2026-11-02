#include "StdInc.h"

#include "TaskComplexPartner.h"
#include "TaskComplexPartnerChat.h"
#include "TaskSimpleStandStill.h"

void CTaskComplexPartner::InjectHooks() {
    RH_ScopedVirtualClass(CTaskComplexPartner, 0x870664, 14);
    RH_ScopedCategory("Tasks/TaskTypes");
    RH_ScopedInstall(Constructor, 0x681E70);
    RH_ScopedVMTInstall(CreateFirstSubTask, 0x681F20);
}

// 0x681E70
CTaskComplexPartner::CTaskComplexPartner(const char* commandName, CPed* partner, bool leadSpeaker, float distanceMultiplier, bool makePedAlwaysFacePartner, int8 updateDirectionCount, CVector point)
    : CTaskComplex()
{
    m_leadSpeaker              = leadSpeaker;
    m_makePedAlwaysFacePartner = makePedAlwaysFacePartner;
    m_distanceMultiplier       = distanceMultiplier;
    m_updateDirectionCount     = updateDirectionCount;
    m_point                    = point;
    m_partner                  = partner;
    m_partnerState             = PARTNER_STATE_UNK_1;
    m_taskCompleted            = false;
    m_firstToTargetFlag        = -1;
    m_requiredAnimsStreamedIn  = false;
    m_animBlockName[0]         = '\0';
    CEntity::SafeRegisterRef(m_partner);
}

/*!
 * @addr 0x683A40
 */
CTaskComplexPartner::~CTaskComplexPartner() {
    CEntity::SafeCleanUpRef(m_partner);

    if (m_requiredAnimsStreamedIn) {
        if (strcmp(m_animBlockName, "") != 0) {
            CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex(m_animBlockName));
        }
        m_requiredAnimsStreamedIn = false;
    }
}

CTaskComplexPartner* CTaskComplexPartner::Constructor(const char* commandName, CPed* partner, bool leadSpeaker, float distanceMultiplier, bool makePedAlwaysFacePartner, int8 updateDirectionCount, CVector point) {
    this->CTaskComplexPartner::CTaskComplexPartner(commandName, partner, leadSpeaker, distanceMultiplier, makePedAlwaysFacePartner, updateDirectionCount, point);
    return this;
}

CTask* CTaskComplexPartner::CreateNextSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x683AD0, CTaskComplexPartner*, CPed*>(this, ped);
}

// 0x681F20
CTask* CTaskComplexPartner::CreateFirstSubTask(CPed* ped) {
    // Like in the original, the base class sets up the conversation of `CTaskComplexPartnerChat`
    if (m_leadSpeaker && m_taskId == TASK_COMPLEX_PARTNER_CHAT) {
        auto* const chat = static_cast<CTaskComplexPartnerChat*>(this);
        if (chat->m_conversationEnabled) {
            if (CAEPedSpeechAudioEntity::RequestPedConversation(ped, m_partner)) {
                chat->m_pedConversationLoaded = true;
            } else if (chat->field_75) {
                chat->m_conversationEnabled = false; // Go on without the conversation
            } else {
                return nullptr;
            }
        }
    }
    ped->StopPlayingHandSignal();
    return new CTaskSimpleStandStill(50, false, false, 8.0f);
}

CTask* CTaskComplexPartner::ControlSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x6840D0, CTaskComplexPartner*, CPed*>(this, ped);
}

void CTaskComplexPartner::StreamRequiredAnims() {
    return plugin::CallMethod<0x682310, CTaskComplexPartner*>(this);
}

void CTaskComplexPartner::RemoveStreamedAnims() {
    return plugin::CallMethod<0x682370, CTask*>(this);
}
