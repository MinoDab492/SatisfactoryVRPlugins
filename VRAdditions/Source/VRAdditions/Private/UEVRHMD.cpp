﻿#include "UEVRHMD.h"

#include "Logging.h"

const int MAX_DEVICE_ID = 2;

UEVRHMD::UEVRHMD() : FHeadMountedDisplayBase(nullptr) {}

void UEVRHMD::SetUEVRParams(UEVR_PluginInitializeParam* Params)
{
	this->UEVRParams = Params;
}

bool UEVRHMD::IsStereoEnabled() const
{
	return true;
}

bool UEVRHMD::EnableStereo(bool stereo)
{
	return true;
}

void UEVRHMD::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
}

FMatrix UEVRHMD::GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType) const
{
	return FMatrix::Identity;

	/*const float ProjectionCenterOffset = 0.151976421f;
	const float PassProjectionOffset = (StereoPassType == eSSP_LEFT_EYE) ? ProjectionCenterOffset : -ProjectionCenterOffset;

	const float HalfFov = 2.19686294f / 2.f;
	const float InWidth = 640.f;
	const float InHeight = 480.f;
	const float XS = 1.0f / tan(HalfFov);
	const float YS = InWidth / tan(HalfFov) / InHeight;

	const float InNearZ = GNearClippingPlane;
	return FMatrix(
		FPlane(XS,                      0.0f,								    0.0f,							0.0f),
		FPlane(0.0f,					YS,	                                    0.0f,							0.0f),
		FPlane(0.0f,	                0.0f,								    0.0f,							1.0f),
		FPlane(0.0f,					0.0f,								    InNearZ,						0.0f))

		* FTranslationMatrix(FVector(PassProjectionOffset,0,0));*/
}

bool UEVRHMD::IsHMDConnected()
{
	return IsHMDEnabled();
}

bool UEVRHMD::IsHMDEnabled() const
{
	auto* VR = GetVR();
	return VR && VR->is_hmd_active();
}

void UEVRHMD::EnableHMD(bool bEnable)
{
}

bool UEVRHMD::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
	MonitorDesc.MonitorName = "";
	MonitorDesc.MonitorId = 0;
	MonitorDesc.DesktopX = MonitorDesc.DesktopY = MonitorDesc.ResolutionX = MonitorDesc.ResolutionY = 0;
	return false;
}

void UEVRHMD::GetFieldOfView(float& InOutHFOVInDegrees, float& InOutVFOVInDegrees) const
{
	InOutHFOVInDegrees = 0.0f;
	InOutVFOVInDegrees = 0.0f;
}

void UEVRHMD::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
}

float UEVRHMD::GetInterpupillaryDistance() const
{
	return 0.064f;
}

bool UEVRHMD::IsChromaAbCorrectionEnabled() const
{
	return true;
}

FName UEVRHMD::GetSystemName() const
{
	return FName("UEVR");
}

int32 UEVRHMD::GetXRSystemFlags() const
{
	return EXRSystemFlags::IsHeadMounted;
}

EXRTrackedDeviceType UEVRHMD::GetTrackedDeviceType(int32 DeviceId) const
{
	switch (DeviceId)
	{
	case 0:
		return EXRTrackedDeviceType::HeadMountedDisplay;
	case 1:
	case 2:
		return EXRTrackedDeviceType::Controller;
	default:
		return EXRTrackedDeviceType::Invalid;
	}
}

bool UEVRHMD::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type)
{
	OutDevices.Empty();

	for (int i = 0; i < MAX_DEVICE_ID; ++i)
	{
		if (Type == EXRTrackedDeviceType::Any || GetTrackedDeviceType(i) == Type)
		{
			OutDevices.Add(i);
		}
	}

	return OutDevices.Num() > 0;
}

bool UEVRHMD::GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition)
{
	if (DeviceId > MAX_DEVICE_ID || !IsInitialized())
	{
		return false;
	}

	UEVR_Vector3f Position;
	UEVR_Quaternionf Orientation;
	GetVR()->get_pose(DeviceId, &Position, &Orientation);
	OutOrientation = FQuat{ Orientation.x, Orientation.y, Orientation.z, Orientation.w };
	OutPosition = FVector{ Position.x, Position.y, Position.z };
	return true;
}

float UEVRHMD::GetWorldToMetersScale() const
{
	return 100.0f;
}

void UEVRHMD::ResetOrientationAndPosition(float Yaw)
{
}

const UEVR_VRData* UEVRHMD::GetVR() const
{
	if (UEVRParams)
	{
		return UEVRParams->vr;
	}
	return nullptr;
}

bool UEVRHMD::IsInitialized() const
{
	return UEVRParams && GetVR()->is_hmd_active();
}