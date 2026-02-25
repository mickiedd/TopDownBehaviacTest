// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviacTypes.h" // For EBehaviacStatus
#include "BehaviacAINPC.generated.h"

class UBehaviacAgentComponent;
class UBehaviacBehaviorTree;

/**
 * AI NPC Character controlled by Behaviac behavior tree
 */
UCLASS()
class TOPDOWNBEHAVIACTEST_API ABehaviacAINPC : public ACharacter
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

	// Property accessors for Behaviac
	UFUNCTION(BlueprintCallable, Category = "AI|Behaviac")
	void SetBehaviacProperty(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "AI|Behaviac")
	FString GetBehaviacProperty(const FString& Key);

private:
	// Patrol points
	TArray<FVector> PatrolPoints;
	int32 CurrentPatrolIndex;
};
