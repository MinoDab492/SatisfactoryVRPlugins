#include "VRAdditionsModule.h"
#include "Engine.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "IHeadMountedDisplayModule.h"
#include <FactoryGame/Public/FGCharacterPlayer.h>

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

struct UEVR_PluginInitializeParam;

void LoadVRInjectorDLL()
{
	// From SteamVRHMD.cpp:
	// OpenVRDLLHandle = FPlatformProcess::GetDllHandle(*(RootOpenVRPath + "UnrealVRBackend.dll"));
	// if (!OpenVRDLLHandle)
	// {
		// UE_LOG(LogHMD, Log, TEXT("Failed to load OpenVR library."));
		// return false;
	// }
	// VRIsHmdPresentFn = (pVRIsHmdPresent)FPlatformProcess::GetDllExport(OpenVRDLLHandle, TEXT("VR_IsHmdPresent"));
	
	const auto unrealVRBackend = GetModuleHandleA("UnrealVRBackend.dll");

	if (unrealVRBackend == nullptr)
	{
		LogToFile("Could not load VR injector dll");
		return;
	}

	auto m_plugin_initialize_param = (UEVR_PluginInitializeParam*)GetProcAddress(unrealVRBackend, "g_plugin_initialize_param");
	LogToFile("VR injector dll loaded");
}

void UnloadVRInjectorDLL()
{
	// if (OpenVRDLLHandle != nullptr)
	// {
		// FPlatformProcess::FreeDllHandle(OpenVRDLLHandle);
		// OpenVRDLLHandle = nullptr;
	// }
}

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

static VRInjectorHeadMountedDisplayModule vrInjectorModule;

void FVRAdditionsModule::StartupModule()
{
	LogToFile("Initialize VRAdditions", false);

	GConfig->SetFloat(TEXT("HMDPluginPriority"), *VR_INJECTOR_NAME, 100, GEngineIni);
	float vrInjectorPriority;
	GConfig->GetFloat(TEXT("HMDPluginPriority"), *VR_INJECTOR_NAME, vrInjectorPriority, GEngineIni);
	LogToFile("VR injector priority: " + FString::SanitizeFloat(vrInjectorPriority));

	LoadVRInjectorDLL();
	
	FName Type = IHeadMountedDisplayModule::GetModularFeatureName();
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.RegisterModularFeature(Type, &vrInjectorModule);

	// Test that it was added:
	LogToFile("HMD Modules:");
	TArray<IHeadMountedDisplayModule*> HMDModules = ModularFeatures.GetModularFeatureImplementations<IHeadMountedDisplayModule>(Type);
	for (IHeadMountedDisplayModule* HMDModule : HMDModules)
	{
		LogToFile("\tFound HMD module " + HMDModule->GetModuleKeyName());
	}

// #if !WITH_EDITOR // if editor has trouble launching, surround hook calls with this
// #endif
	
	// SUBSCRIBE_METHOD(AFGCharacterPlayer::EquipEquipment, [](auto& scope, AFGCharacterPlayer* self, AFGEquipment* equipment) {
		// LogToFile(FString("Player equipped item"));
	// });
}

void FVRAdditionsModule::ShutdownModule()
{
	UnloadVRInjectorDLL();
}

IMPLEMENT_GAME_MODULE(FVRAdditionsModule, VRAdditions);