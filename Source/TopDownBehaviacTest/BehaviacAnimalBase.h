// BehaviacAnimalBase — base class for Behaviac-driven animal characters.
// Handles BehaviacAgentComponent creation and behavior tree loading.
// Subclasses register their own method handlers and tick the tree.

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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
