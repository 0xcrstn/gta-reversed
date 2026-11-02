#include "StdInc.h"

#include "AttractorScanner.h"

void CAttractorScanner::InjectHooks() {
    RH_ScopedClass(CAttractorScanner);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Clear, 0x5FFF90);
}

// 0x5FFF90
void CAttractorScanner::Clear() {
    constexpr auto RANGE       = 5.0f;
    constexpr auto RANGE_LARGE = 15.0f; // 0x86C52C

    for (auto&& [type, rangeSq] : rngv::enumerate(field_68)) {
        field_18[type] = 0;
        field_40[type] = 0;
        rangeSq        = type == PED_ATTRACTOR_SHELTER || type == PED_ATTRACTOR_SCRIPTED
            ? sq(RANGE_LARGE)
            : sq(RANGE);
    }
}
