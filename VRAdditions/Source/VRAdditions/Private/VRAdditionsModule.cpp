#include "VRAdditionsModule.h"
#include "Engine.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "IHeadMountedDisplayModule.h"
#include <FactoryGame/Public/FGCharacterPlayer.h>

#include "FGGameEngine.h"
#include "UEVRHMD.h"
#include "Logging.h"
#include "Math.h"
#include "uevr_api/API.h"

static const FString UEVR_NAME = "UEVR";

static UEVR_PluginInitializeParam* UEVRParams = nullptr;
TSharedPtr<UEVRHMD, ESPMode::ThreadSafe> HMD;
bool IsRunning = true;

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
		Thread = nullptr;
	}

	virtual uint32 Run()
	{
		while (!UEVRParams && IsRunning)
		{
			const HMODULE UnrealVRBackend = GetModuleHandleA("UnrealVRBackend.dll");
			UEVRParams = (UEVR_PluginInitializeParam*) GetProcAddress(UnrealVRBackend, "g_plugin_initialize_param");

			if (!UEVRParams)
			{
				LogToFile("Waiting for UEVR DLL...");
				FPlatformProcess::Sleep(2);
			}
		}

		if (UEVRParams)
		{
			LogToFile("UEVR dll loaded!");
			HMD->SetUEVRParams(UEVRParams);

			while (!UEVRParams->vr->is_hmd_active())
			{
				LogToFile("Waiting for HMD...");
				FPlatformProcess::Sleep(2);
			}

			LogToFile("HMD is active!");
		}

		LogToFile("UEVR DLL loader thread ended");
		return 0;
	}
};
static UEVRDllLoader* UEVRLoader = nullptr;

void FVRAdditionsModule::StartupModule()
{
	LogToFile("Initialize VRAdditions", false);

	GConfig->SetFloat(TEXT("HMDPluginPriority"), *UEVR_NAME, 100, GEngineIni);

	FName Type = IHeadMountedDisplayModule::GetModularFeatureName();
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.RegisterModularFeature(Type, &UEVRHmdModule);

	UEVRLoader = new UEVRDllLoader();

	// AFGCharacterPlayer* CharacterPlayer = GetMutableDefault<AFGCharacterPlayer>();
	UFGGameEngine* GameEngine = GetMutableDefault<UFGGameEngine>();
	// SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::BeginPlay, CharacterPlayer, [](AFGCharacterPlayer* self) {
		// UCameraComponent* PlayerCamera = self->GetCameraComponent();
		// if (self->IsLocallyControlled() && PlayerCamera->bLockToHmd)
		// {
			// LogToFile("Disable camera lock to HMD");
			// PlayerCamera->bLockToHmd = false;
		// }
	// });

	// SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::Tick, CharacterPlayer, [](AFGCharacterPlayer* self, float deltaTime) {
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGGameEngine::Tick, GameEngine, [](UFGGameEngine* self, float deltaSeconds, bool idleMode ) {
		if (!UEVRParams /*|| !self->IsLocallyControlled()*/)
		{
			return;
		}

		const UEVR_VRData* VR = UEVRParams->vr;
		UEVR_Vector3f Position;
		UEVR_Quaternionf Rotation;
		VR->get_pose(VR->get_hmd_index(), &Position, &Rotation);
		UEVR_Quaternionf UEVRCameraRotation = InverseUEVRQuat(Rotation);
		VR->set_rotation_offset(&UEVRCameraRotation);
	});
}

void FVRAdditionsModule::ShutdownModule()
{
	IsRunning = false;
	delete UEVRLoader;
	LogToFile("Module shut down");
}

IMPLEMENT_GAME_MODULE(FVRAdditionsModule, VRAdditions);