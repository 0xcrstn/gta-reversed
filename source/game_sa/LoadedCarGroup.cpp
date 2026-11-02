/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#include "StdInc.h"

#include "LoadedCarGroup.h"

constexpr auto SENTINEL_VALUE_OF_UNUSED = (int16)(MODEL_INVALID);

// The original treats any negative model ID as an unused slot, not just `SENTINEL_VALUE_OF_UNUSED`
static bool IsUnusedSlot(int16 model) {
    return model < 0;
}

void CLoadedCarGroup::InjectHooks() {
    RH_ScopedClass(CLoadedCarGroup);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Clear, 0x611B90);
    RH_ScopedInstall(AddMember, 0x611BB0);
    RH_ScopedInstall(RemoveMember, 0x611BD0);
    RH_ScopedInstall(GetMember, 0x611C20);
    RH_ScopedInstall(CountMembers, 0x611C30);
    RH_ScopedInstall(PickRandomCar, 0x611C50);
    RH_ScopedInstall(SortBasedOnUsage, 0x611E10);
    RH_ScopedInstall(PickLeastUsedModel, 0x611E90);
}

// 0x611E10
void CLoadedCarGroup::SortBasedOnUsage() {
    // Sort from higher to lower usage. The original is a bubble sort, so models used equally keep their order,
    // and it compares `m_nTimesUsed` as a signed byte
    rng::stable_sort(GetAllModels(), std::greater<>{}, [](int16 modelid) {
        return (int8)(CModelInfo::GetVehicleModelInfo(modelid)->m_nTimesUsed);
    });
}

// 0x611BD0
void CLoadedCarGroup::RemoveMember(eModelID modelIndex) {
    if (notsa::remove_first(m_models, (int16)(modelIndex))) {
        m_models.back() = SENTINEL_VALUE_OF_UNUSED;
    }
}

// 0x611C50
eModelID CLoadedCarGroup::PickRandomCar(bool bNotTooManyInTheWorld, bool bOnlyPickNormalCars) {
    if (Empty()) {
        return MODEL_INVALID;
    }

    const auto PickRandom = [&](auto&& choices) {
        if (rng::empty(choices)) {
            return MODEL_INVALID;
        }

        // The original reads `m_nFrq` as signed
        const auto weightSum = notsa::accumulate(choices, 0, [](int16 model) {
            return (int32)(int16)(CModelInfo::GetVehicleModelInfo(model)->m_nFrq);
        });
        if (weightSum == 0) {
            return MODEL_INVALID;
        }

        for (auto tr{ 0 }; tr < 10; tr++) { // tr = tries
            // First, pick a model
            const auto pickedModel = [&] {
                auto pickedWeight = CGeneral::GetRandomNumberInRange(0, weightSum);
                for (auto modelId : choices) {
                    const auto thisModelFrq = (int32)(int16)(CModelInfo::GetVehicleModelInfo(modelId)->m_nFrq);
                    if (thisModelFrq >= pickedWeight) {
                        return (eModelID)(modelId);
                    }
                    pickedWeight -= thisModelFrq;
                }
                // No frequency of any model in the array was `>=` than `pickedWeight`
                // Originally in this case the last value from `choices` was used, but most likely unintentionally
                NOTSA_UNREACHABLE();
            }();

            // Check if it's suitable
            if (   !CTheScripts::HasCarModelBeenSuppressed(pickedModel)
                && !CTheScripts::HasVehicleModelBeenBlockedByScript(pickedModel)
                && !CStreaming::WeAreTryingToPhaseVehicleOut(pickedModel)
                && (!bNotTooManyInTheWorld || CModelInfo::GetVehicleModelInfo(pickedModel)->m_nRefCount <= 2)
            ) {
                return pickedModel;
            }
        }

        // 10 tries, but no luck
        return MODEL_INVALID;
    };

    if (bOnlyPickNormalCars) {
        // Originally an array was created here, and that was used
        // That is more performant, but I doubt that this is so performance critical to care about that.
        return PickRandom(
            GetAllModels() | rng::views::filter([](int16 modelId) {
                switch (CModelInfo::GetVehicleModelInfo(modelId)->m_nVehicleClass) {
                case VEHICLE_CLASS_NORMAL:
                case VEHICLE_CLASS_POORFAMILY:
                case VEHICLE_CLASS_RICHFAMILY:
                case VEHICLE_CLASS_MOTORBIKE:
                    return true;
                }
                return false;
            })
        );
    } else {
#ifdef FIX_BUGS
        return PickRandom(GetAllModels());
#else
        return PickRandom(m_models);
#endif
    }
}

// 0x611E90
eModelID CLoadedCarGroup::PickLeastUsedModel(int32 maxTimesUsed) {
    // Primary criteria is `m_nRefCount`, secondary is `m_nTimesUsed`.
    // The search starts from 999 for both, and they're compared as `int16` and `int8` respectively
    auto  leastUsedModel = MODEL_INVALID;
    int32 minRefCount    = 999;
    int32 minTimesUsed   = 999;
    for (const auto model : GetAllModels()) {
        const auto* const mi        = CModelInfo::GetVehicleModelInfo(model);
        const int32       refCount  = (int16)(mi->m_nRefCount);
        const int32       timesUsed = (int8)(mi->m_nTimesUsed);
        if (refCount < minRefCount || (refCount == minRefCount && timesUsed < minTimesUsed)) {
            leastUsedModel = (eModelID)(model);
            minRefCount    = refCount;
            minTimesUsed   = timesUsed;
        }
    }
    return minTimesUsed <= maxTimesUsed
        ? leastUsedModel
        : MODEL_INVALID;
}

// 0x611C20
eModelID CLoadedCarGroup::GetMember(uint32 idx) const {
    assert(idx < CountMembers());
    return (eModelID)(m_models[idx]);
}

// 0x611C30
uint32 CLoadedCarGroup::CountMembers() const {
    return (uint32)(rng::distance(m_models.begin(), rng::find_if(m_models, IsUnusedSlot)));
}

// NOTSA
bool CLoadedCarGroup::Empty() const {
    return m_models.front() == SENTINEL_VALUE_OF_UNUSED;
}

// 0x611B90
void CLoadedCarGroup::Clear() {
    rng::fill(m_models, SENTINEL_VALUE_OF_UNUSED);
}

// 0x611BB0
void CLoadedCarGroup::AddMember(eModelID member) {
    const auto end = rng::find_if(m_models, IsUnusedSlot);
    if (end != m_models.end()) {
        *end = (int16)(member);
    } else {
        NOTSA_LOG_DEBUG("Failed to add model to group [Out of memory]");
    }
}
