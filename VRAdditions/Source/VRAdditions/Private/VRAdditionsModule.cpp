#include "VRAdditionsModule.h"
#include "Engine.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "IHeadMountedDisplayModule.h"
#include <FactoryGame/Public/FGCharacterPlayer.h>
#include "UEVRHMD.h"
#include "Logging.h"
#include "uevr_api/API.h"

static const FString UEVR_NAME = "UEVR";

static UEVR_PluginInitializeParam* UEVRParams;
TSharedPtr<UEVRHMD, ESPMode::ThreadSafe> HMD;

class UEVRHeadMountedDisplayModule : public IHeadMountedDisplayModule
{
public:
	virtual FString GetModuleKeyName() const override
	{
		return UEVR_NAME;
	}

	virtual bool IsHMDConnected() override
	{
		return true;
	}

	virtual TSharedPtr<IXRTrackingSystem, ESPMode::ThreadSafe> CreateTrackingSystem() override
	{
		HMD = MakeShared<UEVRHMD, ESPMode::ThreadSafe>();
		return HMD;
	}
};

static UEVRHeadMountedDisplayModule UEVRHmdModule;

class UEVRDllLoader : public FRunnable
{
	FRunnableThread* Thread;

public:
	UEVRDllLoader()
	{
		Thread = FRunnableThread::Create(this, TEXT("UEVRDllLoader"));
	}

	~UEVRDllLoader()
	{
		delete Thread;
	}

	virtual uint32 Run()
	{
		while (UEVRParams == nullptr)
		{
			const HMODULE UnrealVRBackend = GetModuleHandleA("UnrealVRBackend.dll");
			UEVRParams = (UEVR_PluginInitializeParam*) GetProcAddress(UnrealVRBackend, "g_plugin_initialize_param");

			if (!UEVRParams)
			{
				LogToFile("Waiting for UEVR DLL...");
				FPlatformProcess::Sleep(2);
			}
		}
		LogToFile("UEVR dll loaded!");
		HMD->SetUEVRParams(UEVRParams);

		while (!UEVRParams->vr->is_hmd_active())
		{
			LogToFile("Waiting for HMD...");
			FPlatformProcess::Sleep(2);
		}

		LogToFile("HMD is active!");

		return 0;
	}
};
static UEVRDllLoader* UEVRLoader;

void FVRAdditionsModule::StartupModule()
{
	LogToFile("Initialize VRAdditions", false);

	GConfig->SetFloat(TEXT("HMDPluginPriority"), *UEVR_NAME, 100, GEngineIni);

	FName Type = IHeadMountedDisplayModule::GetModularFeatureName();
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.RegisterModularFeature(Type, &UEVRHmdModule);

	UEVRLoader = new UEVRDllLoader();

// #if !WITH_EDITOR // if editor has trouble launching, surround hook calls with this
// #endif

	// SUBSCRIBE_METHOD(AFGCharacterPlayer::EquipEquipment, [](auto& scope, AFGCharacterPlayer* self, AFGEquipment* equipment) {
		// LogToFile(FString("Player equipped item"));
	// });
}

void FVRAdditionsModule::ShutdownModule()
{
	delete UEVRLoader;
}

IMPLEMENT_GAME_MODULE(FVRAdditionsModule, VRAdditions);