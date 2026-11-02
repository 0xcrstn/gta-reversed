#pragma once
#include "eRadioID.h"
#include <General.h>

class CVehicle;

static constexpr float VOLUME_SILENCE = -100.0f;

class CAEAudioUtility {
public:
    static void      StaticInitialise();

    /*!
    * @brief Returns a number in [min, max] (unlike CGeneral's, `max` is included)
    */
    // 0x4D9C10
    template<std::integral T>
    static T GetRandomNumberInRange(T min, T max) {
        const auto rnd = static_cast<float>(CGeneral::GetRandomNumber()) * (1.0f / 32768.0f); // [0, 1)
        return static_cast<T>(min + static_cast<int32>(rnd * static_cast<float>(max - min + 1)));
    }

    template<typename T>
        requires std::is_floating_point_v<T>
    static T GetRandomNumberInRange(T min, T max) {
        return CGeneral::GetRandomNumberInRange<T>(min, max);
    }

    static CVehicle* FindVehicleOfPlayer();
    static bool      ResolveProbability(float prob);
    static float     AudioLog10(float p);

    static uint32    ConvertFromBytesToMS(uint32 lengthInBytes, uint32 sampleRate, uint16 numChannels);
    static uint32    ConvertFromMSToBytes(uint32 a, uint32 frequency, uint16 frequencyMult);

    static bool      GetBankAndSoundFromScriptSlotAudioEvent(const eAudioEvents& scriptID, eSoundBankS32& outBankID, int32& outSoundID, int32 slot);
    static float     GetPiecewiseLinear(float x, int16 dataCount, const float (*data)[2]);
    template<int16 DataCnt>
    static float     GetPiecewiseLinearT(float x, const float (&points)[DataCnt][2]) { return GetPiecewiseLinear(x, DataCnt, points); }
    static uint64    GetCurrentTimeInMS();

    // NOTSA
    static eRadioID GetRandomRadioStation() {
        return static_cast<eRadioID>(GetRandomNumberInRange(1, RADIO_COUNT - 1));
    }

private:
    friend void InjectHooksMain();
    static void InjectHooks();
};
