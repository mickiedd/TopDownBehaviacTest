// BehaviacAnimalBase — base class for Behaviac-driven animal characters.
// Handles BehaviacAgentComponent creation, BT loading, and JS-safe nav primitives.
//
// Nav API (all scalar — safe across Puerts/QuickJS boundary):
//   SetNavTarget(x, y)          — store desired destination
//   NavMoveToTarget(acceptance) — issue MoveToLocation; returns 0=Running,1=Success,2=Failure
//   NavStop()                   — stop movement, clear target
//   IsNavTargetSet()            — true if SetNavTarget was called
//   IsNavTargetReached(r)       — true if within radius r of stored target
//   GetNavTargetX/Y()           — read stored target coordinates
//   GetLocationX/Y()            — owner position (avoids FVector boundary)
//   GetSpeedXY()                — 2D speed magnitude

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviacAgent.h"
#include "BehaviacTypes.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "BehaviacAnimalBase.generated.h"

UCLASS()
class TOPDOWNBEHAVIACTEST_API ABehaviacAnimalBase : public ACharacter
{
	GENERATED_BODY()

public:
	ABehaviacAnimalBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviac AI")
	UBehaviacBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviac AI")
	FString BehaviorTreeAssetPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behaviac AI", meta = (AllowPrivateAccess = "true"))
	UBehaviacAgentComponent* BehaviacAgent;

	// ── Position (avoids FVector crossing Puerts boundary) ───────────────────
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetLocationX() const { return GetActorLocation().X; }

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetLocationY() const { return GetActorLocation().Y; }

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetLocationZ() const { return GetActorLocation().Z; }

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetSpeedXY() const   { return GetVelocity().Size2D(); }

	// ── Nav target storage ────────────────────────────────────────────────────
	/** Store a 2D navigation destination. Z is projected onto the navmesh. */
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	void SetNavTarget(float X, float Y);

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetNavTargetX() const { return NavTargetX; }

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	float GetNavTargetY() const { return NavTargetY; }

	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	bool IsNavTargetSet() const { return bNavTargetSet; }

	/** True if the actor is within AcceptanceRadius of the stored nav target. */
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	bool IsNavTargetReached(float AcceptanceRadius) const;

	// ── Nav commands ──────────────────────────────────────────────────────────
	/**
	 * Issue MoveToLocation toward the stored nav target.
	 * Returns: 0 = Running, 1 = Success (arrived), 2 = Failure (no controller / nav fail)
	 * Call every BT tick while MoveToWanderTarget is Running.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	int32 NavMoveToTarget(float AcceptanceRadius);

	/** Stop movement and clear the nav target. */
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	void NavStop();

	/** Set MaxWalkSpeed on the CharacterMovementComponent. */
	UFUNCTION(BlueprintCallable, Category = "AI|Nav")
	void SetMaxSpeed(float Speed);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float NavTargetX    = 0.f;
	float NavTargetY    = 0.f;
	bool  bNavTargetSet = false;
};
