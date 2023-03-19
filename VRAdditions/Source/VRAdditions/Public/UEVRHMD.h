#pragma once
#include "HeadMountedDisplayBase.h"
#include "SceneViewExtension.h"
#include "uevr_api/API.h"

class UEVRHMD : public FHeadMountedDisplayBase
{
public:
	UEVRHMD();
	void SetUEVRParams(UEVR_PluginInitializeParam* UEVRParams);

	virtual bool IsStereoEnabled() const override;
	virtual bool EnableStereo(bool stereo) override;
	virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
	virtual FMatrix GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType) const override;
	virtual bool IsHMDConnected() override;
	virtual bool IsHMDEnabled() const override;
	virtual void EnableHMD(bool bEnable) override;
	virtual bool GetHMDMonitorInfo(MonitorInfo&) override;
	virtual void GetFieldOfView(float& InOutHFOVInDegrees, float& InOutVFOVInDegrees) const override;
	virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
	virtual float GetInterpupillaryDistance() const override;
	virtual bool IsChromaAbCorrectionEnabled() const override;
	virtual FName GetSystemName() const override;
	virtual int32 GetXRSystemFlags() const override;
	virtual EXRTrackedDeviceType GetTrackedDeviceType(int32 DeviceId) const override;
	virtual bool EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type) override;
	virtual bool GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition) override;
	virtual float GetWorldToMetersScale() const override;
	virtual void ResetOrientationAndPosition(float Yaw) override;

private:
	const UEVR_VRData* GetVR() const;
	bool IsInitialized() const;

	UEVR_PluginInitializeParam* UEVRParams = nullptr;
};