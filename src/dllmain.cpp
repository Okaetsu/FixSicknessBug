#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <cstdint>
#include <memory>
#include <safetyhook.hpp>
#include <Unreal/AActor.hpp>
#include <Unreal/UClass.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/UScriptStruct.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/GameplayStatics.hpp>
#include <Unreal/Property/FBoolProperty.hpp>
#include <Unreal/Property/NumericPropertyTypes.hpp>

#include <Signatures.hpp>
#include <SigScanner/SinglePassSigScanner.hpp>

using namespace RC;
using namespace RC::Unreal;

std::vector<SignatureContainer> SigContainer;
SinglePassScanner::SignatureContainerMap SigContainerMap;

typedef void(__cdecl* TYPE_SetWorkerSick)(UObject*, uint8_t);
static inline TYPE_SetWorkerSick SetWorkerSick_Internal;

// Signature stuff, expect them to break with updates
void BeginScan()
{
    SignatureContainer SetWorkerSick_Signature = [=]() -> SignatureContainer {
        return {
            {{ "48 89 5C 24 10 57 48 83 EC 50 88 91 89 03 00 00 0F B6 FA 48 8B D9"}},
            [=](SignatureContainer& self) {
                void* FunctionPointer = static_cast<void*>(self.get_match_address());

                SetWorkerSick_Internal =
                    reinterpret_cast<TYPE_SetWorkerSick>(FunctionPointer);

                self.get_did_succeed() = true;

                return true;
            },
            [](const SignatureContainer& self) {
                if (!self.get_did_succeed())
                {
                    Output::send<LogLevel::Error>(STR("Failed to find signature for UPalIndividualCharacterParameter::SetWorkerSick\n"));
                }
            }
        };
    }();

    SigContainer.emplace_back(SetWorkerSick_Signature);
    SigContainerMap.emplace(ScanTarget::MainExe, SigContainer);
    SinglePassScanner::start_scan(SigContainerMap);
}

SafetyHookInline SetWorkerSick_Hook{};
void __stdcall SetWorkerSick(UObject* This, uint8_t SicknessType)
{
    SetWorkerSick_Hook.call(This, 0);
}

struct SetPhysicalHealth_Params {
    uint8_t PhysicalHealth;
};

class FixSicknessBug : public RC::CppUserModBase
{
public:
    FixSicknessBug() : CppUserModBase()
    {
        ModName = STR("FixSicknessBug");
        ModVersion = STR("1.0.0");
        ModDescription = STR("");
        ModAuthors = STR("Okaetsu");

        Output::send<LogLevel::Verbose>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~FixSicknessBug() override
    {
    }

    auto on_update() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        Output::send<LogLevel::Verbose>(STR("[{}] loaded successfully!\n"), ModName);

        BeginScan();

        SetWorkerSick_Hook = safetyhook::create_inline(reinterpret_cast<void*>(SetWorkerSick_Internal),
            reinterpret_cast<void*>(SetWorkerSick));
    }
};


#define FIXSICKNESSBUG_API __declspec(dllexport)
extern "C"
{
    FIXSICKNESSBUG_API RC::CppUserModBase* start_mod()
    {
        return new FixSicknessBug();
    }

    FIXSICKNESSBUG_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
