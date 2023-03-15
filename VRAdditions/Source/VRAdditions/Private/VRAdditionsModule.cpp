#include "VRAdditionsModule.h"
#include "Engine.h"

void FVRAdditionsModule::StartupModule() {
	UE_LOG(LogTemp, Warning, TEXT("Hello from C++"));
}

IMPLEMENT_GAME_MODULE(FVRAdditionsModule, VRAdditions);
