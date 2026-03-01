// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuertsBootstrap.h"

APuertsBootstrapActor::APuertsBootstrapActor()
{
    PrimaryActorTick.bCanEverTick = false;
    ScriptModule = TEXT("main");
}

void APuertsBootstrapActor::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[Puerts] Initialising JS environment (QuickJS)..."));

    // FJsEnv constructor takes an optional ScriptRoot (defaults to "JavaScript")
    JsEnv = MakeUnique<PUERTS_NAMESPACE::FJsEnv>();

    if (!JsEnv)
    {
        UE_LOG(LogTemp, Error, TEXT("[Puerts] Failed to create JS environment!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Puerts] Running module: %s"), *ScriptModule);

    // Start() takes a module name (resolved relative to Content/JavaScript/) and optional args
    JsEnv->Start(ScriptModule, TArray<TPair<FString, UObject*>>());
}

void APuertsBootstrapActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (JsEnv)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Puerts] Shutting down JS environment."));
        JsEnv.Reset();
    }

    Super::EndPlay(EndPlayReason);
}
