// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviacAgent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JSAIInterface.generated.h"

/**
 * UJSAIInterface
 *
 * A reusable ActorComponent that exposes primitive AI operations as UFUNCTIONs
 * safe to call from Puerts/TypeScript (no FVector/FRotator struct boundaries).
 *
 * Attach to any Character-based AI actor. Configure:
 *   - DetectionRadius, AttackRange, CombatRange, WalkSpeed, RunSpeed, GuardRadius
 *   - PatrolPoints (set from C++ or Blueprint)
 *
 * Requires a UBehaviacAgentComponent sibling for state/blackboard access.
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TOPDOWNBEHAVIACTEST_API UJSAIInterface : public UActorComponent
{
    GENERATED_BODY()

public:
    UJSAIInterface();

    // ── Config ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float DetectionRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float CombatRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float WalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float RunSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float GuardRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    AActor* TargetActor = nullptr;

    // ── Sensor primitives ────────────────────────────────────────────────────

    /** True if player is within DetectionRadius AND has line-of-sight. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Sensor")
    bool CanSeePlayer() const;

    /** Distance from owner to player pawn. -1 if no player. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Sensor")
    float GetDistanceToPlayer() const;

    /** Distance from owner to TargetActor. -1 if no target. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Sensor")
    float GetDistanceToTarget() const;

    /** Distance from owner to guard post (spawn location). */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Sensor")
    float GetDistanceFromPost() const;

    /** Distance from player to guard post. -1 if no player. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Sensor")
    float GetPlayerDistanceFromPost() const;

    // ── Owner position (avoids FVector boundary) ─────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Position")
    float GetLocationX() const;
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Position")
    float GetLocationY() const;
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Position")
    float GetLocationZ() const;
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Position")
    float GetSpeedXY() const;

    // ── Movement primitives ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void SetSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void StopMovement();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void MoveToTarget();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void MoveToPost();

    /** Move to last known player position. Returns true if arrived. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    bool MoveToLastKnownPos();

    /** Advance to next patrol point. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void Patrol();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void FaceTarget();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Movement")
    void LookAround();

    // ── Goofy actions ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void Jump();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void Crouch();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void UnCrouch();

    /** Launch character upward with given Z force. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void LaunchUp(float ZForce = 600.f);

    /** Spin in place — adds Degrees to current yaw. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void Spin(float Degrees = 45.f);

    /** Dash in current facing direction. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void Dash(float Force = 1200.f);

    /** Direct speed setter (for sprint overrides). */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void SetSpeedRaw(float Speed);

    /** Run directly away from player. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void FleeFromPlayer();

    /** Face away from player. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void FaceAwayFromPlayer();

    /** Spin a random amount (60-360 degrees). */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void RandomSpin();

    /** Launch up + spin 180 — classic taunt. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|Goofy")
    void TauntJump();

    // ── Blackboard / state ───────────────────────────────────────────────────

    /** Set a Behaviac blackboard string property. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|State")
    void SetAIState(const FString& NewState);

    UFUNCTION(BlueprintCallable, Category = "AI|JS|State")
    FString GetAIState() const;

    /** Store player's current position as last known. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|State")
    void SetLastKnownPos();

    /** Clear last known position and nullify target. */
    UFUNCTION(BlueprintCallable, Category = "AI|JS|State")
    void ClearLastKnownPos();

    UFUNCTION(BlueprintCallable, Category = "AI|JS|State")
    bool HasLastKnownPos() const { return bHasLastKnownPos; }

    // ── Patrol point setup ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "AI|JS|Patrol")
    int32 GetPatrolPointCount() const { return PatrolPoints.Num(); }

protected:
    virtual void BeginPlay() override;

private:
    // Cached component references (resolved on BeginPlay)
    UPROPERTY()
    UBehaviacAgentComponent* BehaviacAgent = nullptr;

    // Runtime state
    FVector GuardCenter          = FVector::ZeroVector;
    FVector LastKnownPlayerPos   = FVector::ZeroVector;
    bool    bHasLastKnownPos     = false;
    int32   CurrentPatrolIndex   = 0;
    int32   LookAroundDir        = 1;

    TArray<FVector> PatrolPoints;

    // Helpers
    AAIController*              GetAIC()      const;
    UCharacterMovementComponent* GetMovement() const;
    APawn*                      GetPlayer()   const;
};
