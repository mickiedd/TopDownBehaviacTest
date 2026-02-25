// Behaviac UE5 Plugin
// Licensed under the BSD 3-Clause License.

#include "BehaviacEditorModule.h"
#include "BehaviacBehaviorTreeFactory.h"
#include "BehaviacEditorCommands.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FBehaviacEditorModule"

void FBehaviacEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("BehaviacEditor module started."));
}

void FBehaviacEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("BehaviacEditor module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBehaviacEditorModule, BehaviacEditor)
