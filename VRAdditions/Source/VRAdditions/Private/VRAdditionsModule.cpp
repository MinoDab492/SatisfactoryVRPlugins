#include "VRAdditionsModule.h"
#include "Engine.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "IHeadMountedDisplayModule.h"
#include <FactoryGame/Public/FGCharacterPlayer.h>
#include "uevr_api/API.h"

static const FString UEVR_NAME = "UEVR";

void LogToFile(const FString& text, bool append = true)
{
	static FString fileName("VRAdditions.log");

	FString time = FDateTime::Now().ToString();
	FString textResult = time + ": " + text + FString("\n");
	uint32 flags = append ? FILEWRITE_Append : FILEWRITE_None;
	FFileHelper::SaveStringToFile(textResult,
		*fileName,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		flags);
}

static UEVR_PluginInitializeParam* UEVRParams;

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
		TSharedPtr<IXRTrackingSystem, ESPMode::ThreadSafe> DummyVal = nullptr;
		// TODO bridge to UEVR here
		return DummyVal;
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

		while (!UEVRParams->vr->is_hmd_active())
		{
			LogToFile("Waiting for HMD...");
			FPlatformProcess::Sleep(2);
		}

		LogToFile("HMD is active!");

		int HmdIndex = UEVRParams->vr->get_hmd_index();
		LogToFile("HMD index: " + FString::FromInt(HmdIndex));

		while (true)
		{
			UEVR_Vector3f Position;
			UEVR_Quaternionf Rotation;
			UEVRParams->vr->get_pose(HmdIndex, &Position, &Rotation);
			FString PositionStr = FString::Printf(TEXT("[%f,%f,%f]"), Position.x, Position.y, Position.z);
			LogToFile("HMD position: " + PositionStr);
			FPlatformProcess::Sleep(2);
		}

		return 0;
	}
};
static UEVRDllLoader* UEVRLoader;

void FVRAdditionsModule::StartupModule()
{
	LogToFile("Initialize VRAdditions", false);

	GConfig->SetFloat(TEXT("HMDPluginPriority"), *UEVR_NAME, 100, GEngineIni);
	float uevrPriority;
	GConfig->GetFloat(TEXT("HMDPluginPriority"), *UEVR_NAME, uevrPriority, GEngineIni);
	LogToFile("UEVR priority: " + FString::SanitizeFloat(uevrPriority));

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