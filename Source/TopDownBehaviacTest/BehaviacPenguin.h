// BehaviacPenguin — wandering penguin driven by PenguinWanderTree Behaviac BT.
//
// Architecture:
//   BT XML owns all decisions, timing, branching.
//   TypeScript (penguin_logic.ts) binds directly to BehaviacAgent.OnMethodNameCalled.
//   C++ methods remain available as native implementations callable from TS.
//
// Argv passed to JS:
//   self     → ABehaviacPenguin (actor)
//   btBridge → UPuertsNPCComponent (JS environment bootstrap component)

#pragma once

#include "CoreMinimal.h"
#include "BehaviacAnimalBase.h"
#include "BehaviacAgent.h"
#include "BehaviacTypes.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "PuertsNPCComponent.h"
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

	/** Current mood roll [0,1) — synced to BT blackboard by RollMood() */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Mood")
	float MoodRoll = 0.f;

	/** Puerts JS environment — bridges BT actions to TypeScript */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puerts")
	UPuertsNPCComponent* PuertsComp;

	// ── Sensor helpers ────────────────────────────────────────────────
	// (GetLocationX/Y, GetSpeedXY, SetMaxSpeed inherited from BehaviacAnimalBase)

	UFUNCTION(BlueprintCallable, Category = "AI|Mood")
	float GetMoodRoll() const  { return MoodRoll; }
	UFUNCTION(BlueprintCallable, Category = "AI|Mood")
	void  SetMoodRoll(float V) { MoodRoll = V; BehaviacAgent->SetFloatProperty(TEXT("MoodRoll"), V); }

	// ── BT actions — native implementations, callable from TS ─────────
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus RollMood();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus PickWanderTarget();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus MoveToWanderTarget();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus StopMovement();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus LookAround();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus SetSleepySpeed();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus SetWanderSpeed();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus SetExcitedSpeed();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus MaybeSpin();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus SpinAround();
	UFUNCTION(BlueprintCallable, Category = "AI|Actions") EBehaviacStatus ExcitedJump();

private:
	void UpdateBehaviacProperties();

	// C++ fallback implementations
	EBehaviacStatus CPP_RollMood();
	EBehaviacStatus CPP_LookAround();

	int32   TickCounter        = 0;

	float LookAroundTargetYaw  = 0.f;
	bool  bLookingAround       = false;
	bool  bLookAroundComplete  = false;
};
