// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JsEnv.h"
#include "PuertsNPCComponent.generated.h"

/**
 * UPuertsNPCComponent
 *
 * Attach to any NPC actor to run a per-instance Puerts JS environment.
 * The owning actor is injected into JS as "self" via puerts.argv.
 *
 * Set ScriptModule to the module name (relative to Content/JavaScript/, no ext).
 * Default: "npc_logic"
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TOPDOWNBEHAVIACTEST_API UPuertsNPCComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPuertsNPCComponent();

    /** JS module name to load (Content/JavaScript/<ScriptModule>.js) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puerts")
    FString ScriptModule;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TUniquePtr<PUERTS_NAMESPACE::FJsEnv> JsEnv;
};
