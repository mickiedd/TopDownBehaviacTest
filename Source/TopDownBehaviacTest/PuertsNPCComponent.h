// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviacTypes.h"
#include "JsEnv.h"
#include "PuertsNPCComponent.generated.h"

/** Fired when the BT wants to execute a named action.
 *  JS handler must call btBridge.SetBTResult(n) synchronously:
 *    0 = Running, 1 = Success, 2 = Failure */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBTAction, const FString&, ActionName);

/**
 * UPuertsNPCComponent
 *
 * Boots a per-NPC Puerts JS environment and bridges Behaviac BT actions to JS.
 *
 * Flow:
 *   BT node fires → C++ action calls DispatchBTAction("ActionName")
 *   → OnBTAction delegate fires → JS handler runs synchronously
 *   → JS calls btBridge.SetBTResult(1) → DispatchBTAction returns that value
 *
 * JS setup (in npc_logic.js):
 *   const btBridge = puerts.argv.getByName("btBridge");
 *   btBridge.OnBTAction.Add((actionName) => {
 *       const result = myHandlers[actionName]?.() ?? 0;
 *       btBridge.SetBTResult(result);
 *   });
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

    /**
     * Called by C++ BT action stubs to hand off logic to JS.
     * Fires OnBTAction; JS must call SetBTResult() synchronously.
     * Returns 0 (Running) if no JS handler is bound.
     */
    UFUNCTION(BlueprintCallable, Category = "Puerts|BT")
    int32 DispatchBTAction(const FString& ActionName);

    /**
     * JS calls this to write the result of the current BT action.
     * Must be called synchronously within the OnBTAction handler.
     *   0 = Running, 1 = Success, 2 = Failure
     */
    UFUNCTION(BlueprintCallable, Category = "Puerts|BT")
    void SetBTResult(int32 Result) { PendingResult = Result; }

    /** Fired each time the BT wants to execute an action. JS binds here. */
    UPROPERTY(BlueprintAssignable, Category = "Puerts|BT")
    FOnBTAction OnBTAction;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    TUniquePtr<PUERTS_NAMESPACE::FJsEnv> JsEnv;
    int32 PendingResult = 0;
};
