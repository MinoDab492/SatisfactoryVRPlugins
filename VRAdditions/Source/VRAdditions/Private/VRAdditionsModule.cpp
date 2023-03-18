#include "VRAdditionsModule.h"
#include "Engine.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "IHeadMountedDisplayModule.h"
#include <FactoryGame/Public/FGCharacterPlayer.h>
#include "uevr_api/API.h"

static const FString VR_INJECTOR_NAME = "VRInjector";

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

static UEVR_PluginInitializeParam* VrInjectorParams;

class VRInjectorHeadMountedDisplayModule : public IHeadMountedDisplayModule
{
public:
	virtual FString GetModuleKeyName() const override
	{
		return VR_INJECTOR_NAME;
	}

	virtual bool IsHMDConnected() override
	{
		return true;
	}

	virtual TSharedPtr<IXRTrackingSystem, ESPMode::ThreadSafe> CreateTrackingSystem() override
	{
		TSharedPtr<IXRTrackingSystem, ESPMode::ThreadSafe> DummyVal = nullptr;
		// TODO bridge to VR injector here
		return DummyVal;
	}
};

static VRInjectorHeadMountedDisplayModule VrInjectorModule;

class InjectorDllLoader : public FRunnable
{
	FRunnableThread* Thread;

public:
	InjectorDllLoader()
	{
		Thread = FRunnableThread::Create(this, TEXT("InjectorDllLoader"));
		LogToFile("Started injector DLL loader thread");
	}

	~InjectorDllLoader()
	{
		delete Thread;
	}

	virtual uint32 Run()
	{
		while (VrInjectorParams == nullptr)
		{
			const HMODULE UnrealVRBackend = GetModuleHandleA("UnrealVRBackend.dll");
			VrInjectorParams = (UEVR_PluginInitializeParam*) GetProcAddress(UnrealVRBackend, "g_plugin_initialize_param");

			if (!VrInjectorParams)
			{
				LogToFile("Waiting for VR injector DLL...");
				FPlatformProcess::Sleep(2);
			}
		}
		LogToFile("VR injector dll loaded!");

		while (!VrInjectorParams->vr->is_hmd_active())
		{
			LogToFile("Waiting for HMD...");
			FPlatformProcess::Sleep(2);
		}

		LogToFile("HMD is active!");

		int HmdIndex = VrInjectorParams->vr->get_hmd_index();
		LogToFile("HMD index: " + FString::FromInt(HmdIndex));

		while (true)
		{
			UEVR_Vector3f Position;
			UEVR_Quaternionf Rotation;
			VrInjectorParams->vr->get_pose(HmdIndex, &Position, &Rotation);
			FString PositionStr = FString::Printf(TEXT("[%f,%f,%f]"), Position.x, Position.y, Position.z);
			LogToFile("HMD position: " + PositionStr);
			FPlatformProcess::Sleep(2);
		}

		return 0;
	}
};
static InjectorDllLoader* InjectorLoader;

void FVRAdditionsModule::StartupModule()
{
	LogToFile("Initialize VRAdditions", false);

	GConfig->SetFloat(TEXT("HMDPluginPriority"), *VR_INJECTOR_NAME, 100, GEngineIni);
	float vrInjectorPriority;
	GConfig->GetFloat(TEXT("HMDPluginPriority"), *VR_INJECTOR_NAME, vrInjectorPriority, GEngineIni);
	LogToFile("VR injector priority: " + FString::SanitizeFloat(vrInjectorPriority));

	FName Type = IHeadMountedDisplayModule::GetModularFeatureName();
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.RegisterModularFeature(Type, &VrInjectorModule);

	InjectorLoader = new InjectorDllLoader();

// #if !WITH_EDITOR // if editor has trouble launching, surround hook calls with this
// #endif

	// SUBSCRIBE_METHOD(AFGCharacterPlayer::EquipEquipment, [](auto& scope, AFGCharacterPlayer* self, AFGEquipment* equipment) {
		// LogToFile(FString("Player equipped item"));
	// });
}

void FVRAdditionsModule::ShutdownModule()
{
	delete InjectorLoader;
}

IMPLEMENT_GAME_MODULE(FVRAdditionsModule, VRAdditions);