#pragma once

#include "uevr_api/API.h"

FVector UEVRVecToFVec(const UEVR_Vector3f& UEVRVector)
{
	return FVector{ UEVRVector.z, -UEVRVector.x, -UEVRVector.y };
}

UEVR_Vector3f FVecToUEVRVec(const FVector& Vec)
{
	return UEVR_Vector3f{ -Vec.Y, -Vec.Z, Vec.X };
}

FQuat UEVRQuatToFQuat(const UEVR_Quaternionf& UEVRQuat)
{
	return FQuat{ UEVRQuat.z, -UEVRQuat.x, -UEVRQuat.y, UEVRQuat.w };
}

UEVR_Quaternionf FQuatToUEVRQuat(const FQuat& Quat)
{
	UEVR_Quaternionf Result;
	Result.x = -Quat.Y;
	Result.y = -Quat.Z;
	Result.z = Quat.X;
	Result.w = Quat.W;
	return Result;
}

FQuat Flatten(const FQuat& Quat)
{
	FVector Forward = Quat.GetForwardVector();
	Forward.Normalize();
	FVector FlattenedForward = { Forward.X, Forward.Y, 0.0f };
	FlattenedForward.Normalize();
	return FlattenedForward.Rotation().Quaternion();
}

UEVR_Quaternionf InverseUEVRQuat(const UEVR_Quaternionf& Quat)
{
	FQuat UEQuat = UEVRQuatToFQuat(Quat);
	UEQuat = UEQuat.Inverse();
	return FQuatToUEVRQuat(UEQuat);
}