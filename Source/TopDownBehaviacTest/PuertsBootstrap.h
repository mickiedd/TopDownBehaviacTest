// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JsEnv.h"
#include "PuertsBootstrap.generated.h"

/**
 * APuertsBootstrapActor
 *
 * Drop this actor into any level to initialise the Puerts JS environment and
 * execute Content/JavaScript/main.js on BeginPlay.
 *
 * Set ScriptPath (relative to project Content dir) to point at a different
 * entry-point script if needed.
 */
UCLASS(Blueprintable, BlueprintType)
class TOPDOWNBEHAVIACTEST_API APuertsBootstrapActor : public AActor
{
    GENERATED_BODY()

public:
    APuertsBootstrapActor();

    /** Path to the entry-point JS module name (relative to Content/JavaScript, no extension). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puerts")
    FString ScriptModule;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** Owning JS environment — kept alive for the duration of the actor's life. */
    TUniquePtr<PUERTS_NAMESPACE::FJsEnv> JsEnv;
};
