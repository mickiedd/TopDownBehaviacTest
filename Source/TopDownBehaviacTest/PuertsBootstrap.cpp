// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuertsBootstrap.h"
#include "JsEnv.h"
#include "Misc/Paths.h"

APuertsBootstrapActor::APuertsBootstrapActor()
{
    PrimaryActorTick.bCanEverTick = false;
    ScriptPath = TEXT("JavaScript/main.js");
}

void APuertsBootstrapActor::BeginPlay()
{
    Super::BeginPlay();

    // Resolve absolute path: <ProjectDir>/Content/<ScriptPath>
    FString FullPath = FPaths::ProjectContentDir() / ScriptPath;
    FPaths::NormalizeFilename(FullPath);

    UE_LOG(LogTemp, Warning, TEXT("[Puerts] Initialising JS environment..."));
    UE_LOG(LogTemp, Warning, TEXT("[Puerts] Script: %s"), *FullPath);

    // Create the JS environment (QuickJS backend)
    JsEnv = MakeShared<puerts::FJsEnv>();

    if (!JsEnv.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Puerts] Failed to create JS environment!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Puerts] JS environment ready. Running script..."));

    // Execute the entry-point script
    JsEnv->Start(ScriptPath, {});
}

void APuertsBootstrapActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (JsEnv.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Puerts] Shutting down JS environment."));
        JsEnv.Reset();
    }

    Super::EndPlay(EndPlayReason);
}
