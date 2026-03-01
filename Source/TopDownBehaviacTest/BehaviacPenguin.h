// BehaviacPenguin — wandering penguin driven by the PenguinWanderTree Behaviac BT.
//
// Behavior cycle (PenguinWanderTree.xml):
//   1. PickWanderTarget  – choose random point near spawn
//   2. MoveToWanderTarget – walk there (Running until arrived)
//   3. StopMovement       – halt navigation
//   4. Wait 1.5 s
//   5. LookAround         – smooth yaw rotation
//   6. Wait 3.0 s
//   Loop forever (DecoratorLoop Count=-1)

#pragma once

#include "CoreMinimal.h"
#include "BehaviacAnimalBase.h"
#include "BehaviacAgent.h"
#include "BehaviacTypes.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "BehaviacPenguin.generated.h"

UCLASS()
class TOPDOWNBEHAVIACTEST_API ABehaviacPenguin : public ABehaviacAnimalBase
{
	GENERATED_BODY()

public:
	ABehaviacPenguin();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ── Wander tuning ──────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander")
	float WanderRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander")
	float WanderSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander")
	float WanderAcceptanceRadius = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Wander")
	float TurnInterpSpeed = 120.f;

	/** Current mood roll [0,1) set by RollMood each cycle — read by BT Conditions */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Mood")
	float MoodRoll = 0.f;

	// ── BT actions ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus RollMood();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus PickWanderTarget();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus MoveToWanderTarget();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus StopMovement();

	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus LookAround();

	/** Set speed to sleepy shuffle (60% of WanderSpeed) */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SetSleepySpeed();

	/** Reset to normal WanderSpeed */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SetWanderSpeed();

	/** Set speed to excited sprint (2.5x WanderSpeed) */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SetExcitedSpeed();

	/** 40% chance to do a random spin, always returns Success */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus MaybeSpin();

	/** Full 360° celebratory spin */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus SpinAround();

	/** Launch up with a happy jump */
	UFUNCTION(BlueprintCallable, Category = "AI|Actions")
	EBehaviacStatus ExcitedJump();

private:
	void UpdateBehaviacProperties();

	FVector SpawnLocation;
	FVector WanderTarget;
	bool    bHasWanderTarget = false;
	int32   TickCounter      = 0;

	float LookAroundTargetYaw  = 0.f;
	bool  bLookingAround       = false;
	bool  bLookAroundComplete  = false;
};
