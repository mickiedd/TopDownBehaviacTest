// BehaviacPenguin — wandering penguin driven by PenguinWanderTree Behaviac BT.
//
// Architecture:
//   BT XML owns all decisions, timing, branching.
//   TypeScript (penguin_logic.ts) owns all BT leaf-action behavior.
//   C++ only provides actor state, nav primitives, and BT property sync helpers.
//
// Argv passed to JS:
//   self     → ABehaviacPenguin (actor)
//   btBridge → UPuertsNPCComponent (JS environment bootstrap component)

#pragma once

#include "CoreMinimal.h"
#include "BehaviacAnimalBase.h"
#include "BehaviacAgent.h"
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
	void  SetMoodRoll(float V)
	{
		MoodRoll = V;
		if (BehaviacAgent)
		{
			BehaviacAgent->SetFloatProperty(TEXT("MoodRoll"), V);
		}
	}

private:
	void UpdateBehaviacProperties();
};
