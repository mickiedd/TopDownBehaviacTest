// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TopDownBehaviacTestCharacter.h"
#include "BehaviacTypes.h" // For EBehaviacStatus
#include "PuertsNPCComponent.h"
#include "JSAIInterface.h"
#include "BehaviacAINPC.generated.h"

class UBehaviacAgentComponent;
class UBehaviacBehaviorTree;

/**
 * AI NPC Character controlled by Behaviac behavior tree
 */
UCLASS()
class TOPDOWNBEHAVIACTEST_API ABehaviacAINPC : public ATopDownBehaviacTestCharacter
{
	GENERATED_BODY()

public:
	ABehaviacAINPC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Behaviac Agent Component - The AI brain
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Behaviac")
	UBehaviacAgentComponent* BehaviacAgent;

	// Puerts JS bridge component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Puerts")
	UPuertsNPCComponent* PuertsNPC;

	// JS AI primitive interface — reusable by any actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Puerts")
	UJSAIInterface* JSAI;

	// Behavior tree to load on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behaviac")
	UBehaviacBehaviorTree* BehaviorTree;

	// AI Properties (Blackboard values)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float DetectionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float RunSpeed;

	// Target player reference
	UPROPERTY(BlueprintReadOnly, Category = "AI|State")
	AActor* TargetPlayer;

	// AI Actions (called by behavior tree) - Must return EBehaviacStatus for Behaviac
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus FindPlayer();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus MoveToTarget();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus Patrol();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	void Idle();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	bool IsPlayerInRange();

	// ── New actions for BT_PatrolGuard ──────────────────────────────────

	/** Scan surroundings, update AIState blackboard property:
	 *  "Combat" / "Chase" / "Investigate" / "Patrol" */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus UpdateAIState();

	/** Sprint toward TargetPlayer. Returns Running while moving, Success when close. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus ChasePlayer();

	/** Melee/ranged attack. Returns Success always (damage dealt elsewhere). */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus AttackPlayer();

	/** Stop movement immediately. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus StopMovement();

	/** Rotate to face TargetPlayer. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus FaceTarget();

	/** Apply walk speed to CharacterMovement. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SetWalkSpeed();

	/** Apply run speed to CharacterMovement. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SetRunSpeed();

	/** Navigate to LastKnownPlayerPos. Returns Running → Success on arrival. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus MoveToLastKnownPos();

	/** Rotate left/right to simulate looking around. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus LookAround();

	/** Clear LastKnownPlayerPos and reset Investigate state → Patrol. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus ClearLastKnownPos();

	/** Walk back to GuardCenter when player has left GuardRadius. */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus ReturnToPost();

	// Guard ground config (editable in Blueprint/level)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Guard")
	float GuardRadius;

	// Property accessors for Behaviac
	UFUNCTION(BlueprintCallable, Category = "AI|Behaviac")
	void SetBehaviacProperty(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "AI|Behaviac")
	FString GetBehaviacProperty(const FString& Key);

private:
	/**
	 * If PuertsNPC has a JS handler bound, dispatch the action to JS and return its result.
	 * Otherwise falls through to the provided C++ lambda.
	 */
	EBehaviacStatus DispatchOrRun(const FString& ActionName, TFunction<EBehaviacStatus()> CppImpl);

	// Patrol points
	TArray<FVector> PatrolPoints;
	int32 CurrentPatrolIndex;

	// Per-instance tick counter (avoids static-local counter persisting across PIE sessions)
	int32 TickCounter;
	float DebugTimer;

	// Guard state
	FVector LastKnownPlayerPos;
	bool bHasLastKnownPos;
	float LookAroundYaw;       // Accumulated yaw for LookAround
	int32 LookAroundDir;       // +1 or -1
	float AttackRange;         // Range to switch to Combat state
	float CombatRange;         // Range to keep attacking before backing off

	// Guard ground
	FVector GuardCenter;       // Initialized to spawn location in BeginPlay
};
