#pragma once

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