#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

using TInitScriptMemberVariable = void*(void* a1, void* a2, uint64_t a3, uint64_t nameHash, void* a5, void* a6, void* a7);
TInitScriptMemberVariable* RealInitScriptMemberVariable = nullptr;

void* HookInitScriptMemberVariable(void* a1, void* a2, uint64_t a3, uint64_t nameHash, void* a5, void* a6, void* a7)
{
    // Break the nameHash of some SplashScreenLoadingScreenLogicController variables
    // Should prevent the intro screen scripts from finding the intro bink, which makes it show a loading screen instead
    // (intro movie audio still plays though - this can be stopped by disabling more script vars, but unfortunately that'll also make it load infinitely)
    // For me the loading screen takes almost as much time as the intro movie itself did, but the audio shows that a few seconds are saved with this, maybe faster machines can save even more time.

    // Ideally I think the real solution is to change GameFramework/InitialState INI variable from "Initialization" to "PreGameSession" or "MainMenu" instead
    // Unfortunately that causes a black screen on launch though, likely only works properly on non-shipping builds

    if (nameHash == RED4ext::FNV1a("logoTrainWBBink") ||
        nameHash == RED4ext::FNV1a("logoTrainNamcoBink") ||
        nameHash == RED4ext::FNV1a("logoTrainStadiaBink") ||
        nameHash == RED4ext::FNV1a("logoTrainNoRTXBink") ||
        nameHash == RED4ext::FNV1a("logoTrainRTXBink") ||
        nameHash == RED4ext::FNV1a("introMessageBink"))
    {
        nameHash = ~nameHash;
    }

    return RealInitScriptMemberVariable(a1, a2, a3, nameHash, a5, a6, a7);
}

void DisableIntroMoviesPatch(Image* apImage)
{
    RealInitScriptMemberVariable = reinterpret_cast<TInitScriptMemberVariable*>(FindSignature(apImage->pTextStart, apImage->pTextEnd, {
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x44, 0x24, 0x50, 0x48, 0x8B, 0xD9, 0x48, 0x89, 0x41, 0x08
        }));

    if (RealInitScriptMemberVariable == nullptr)
    {
        spdlog::info("Disable intro movies patch: failed, could not be found");
        return;
    }

    MH_CreateHook(RealInitScriptMemberVariable, &HookInitScriptMemberVariable, reinterpret_cast<void**>(&RealInitScriptMemberVariable));
    spdlog::info("Disable intro movies patch: success");
}
