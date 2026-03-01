// Copyright Epic Games, Inc. All Rights Reserved.

#include "PuertsNPCComponent.h"

UPuertsNPCComponent::UPuertsNPCComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ScriptModule = TEXT("npc_logic");
}

void UPuertsNPCComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[PuertsNPC] No owner actor!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PuertsNPC] Starting JS env for: %s (module: %s)"),
        *Owner->GetName(), *ScriptModule);

    JsEnv = MakeUnique<PUERTS_NAMESPACE::FJsEnv>();
    if (!JsEnv)
    {
        UE_LOG(LogTemp, Error, TEXT("[PuertsNPC] Failed to create JS environment!"));
        return;
    }

    // Pass the owning NPC actor into JS as "self"
    TArray<TPair<FString, UObject*>> Args;
    Args.Add(TPair<FString, UObject*>(TEXT("self"), Owner));

    JsEnv->Start(ScriptModule, Args);
}

void UPuertsNPCComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (JsEnv)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PuertsNPC] Shutting down JS env for: %s"),
            *GetOwner()->GetName());
        JsEnv.Reset();
    }
    Super::EndPlay(EndPlayReason);
}
