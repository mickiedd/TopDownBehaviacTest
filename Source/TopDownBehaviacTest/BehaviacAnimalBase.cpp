// BehaviacAnimalBase — see BehaviacAnimalBase.h

#include "BehaviacAnimalBase.h"
#include "BehaviacTypes.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABehaviacAnimalBase::ABehaviacAnimalBase()
{
	BehaviacAgent = CreateDefaultSubobject<UBehaviacAgentComponent>(TEXT("BehaviacAgent"));
	BehaviacAgent->bAutoTick = false;
}

void ABehaviacAnimalBase::BeginPlay()
{
	Super::BeginPlay();

	if (!BehaviacAgent)
	{
		BehaviacAgent = FindComponentByClass<UBehaviacAgentComponent>();
		if (BehaviacAgent)
		{
			BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: BehaviacAgent recovered via FindComponentByClass"), *GetName());
		}
		else
		{
			UE_LOG(LogBehaviac, Error, TEXT("[BehaviacAnimalBase] %s: BehaviacAgent missing — cannot run behavior tree"), *GetName());
			return;
		}
	}

	bool bLoaded = false;
	if (BehaviorTree)
	{
		bLoaded = BehaviacAgent->LoadBehaviorTree(BehaviorTree);
		BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: BT loaded from asset reference"), *GetName());
	}
	else if (!BehaviorTreeAssetPath.IsEmpty())
	{
		BehaviacAgent->LoadBehaviorTreeByPath(BehaviorTreeAssetPath);
		bLoaded = true;
		BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: BT loaded from path"), *GetName());
	}

	if (!bLoaded)
	{
		UE_LOG(LogBehaviac, Error, TEXT("[BehaviacAnimalBase] %s: failed to load behavior tree!"), *GetName());
	}
}

void ABehaviacAnimalBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABehaviacAnimalBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// ── Nav primitives ────────────────────────────────────────────────────────────

void ABehaviacAnimalBase::SetNavTarget(float X, float Y)
{
	NavTargetX    = X;
	NavTargetY    = Y;
	bNavTargetSet = true;
	BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: SetNavTarget (%.0f, %.0f)"), *GetName(), X, Y);
}

bool ABehaviacAnimalBase::IsNavTargetReached(float AcceptanceRadius) const
{
	if (!bNavTargetSet) return false;
	FVector Loc = GetActorLocation();
	float Dx = Loc.X - NavTargetX;
	float Dy = Loc.Y - NavTargetY;
	return (Dx * Dx + Dy * Dy) <= (AcceptanceRadius * AcceptanceRadius);
}

int32 ABehaviacAnimalBase::NavMoveToTarget(float AcceptanceRadius)
{
	if (!bNavTargetSet) return 2; // Failure

	AAIController* AIC = GetController<AAIController>();
	if (!AIC) return 2; // Failure

	if (IsNavTargetReached(AcceptanceRadius))
	{
		AIC->StopMovement();
		return 1; // Success
	}

	FVector Dest(NavTargetX, NavTargetY, GetActorLocation().Z);
	EPathFollowingRequestResult::Type Result = AIC->MoveToLocation(
		Dest, AcceptanceRadius * 0.8f, true, true, true, false);

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)    return 1; // Success
	if (Result == EPathFollowingRequestResult::RequestSuccessful) return 0; // Running

	BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: NavMoveToTarget — pathfinding failed, skipping"), *GetName());
	return 1; // treat nav fail as success to avoid blocking the loop
}

void ABehaviacAnimalBase::NavStop()
{
	if (AAIController* AIC = GetController<AAIController>())
		AIC->StopMovement();
	bNavTargetSet = false;
	BEHAVIAC_VLOG(TEXT("[BehaviacAnimalBase] %s: NavStop"), *GetName());
}

void ABehaviacAnimalBase::SetMaxSpeed(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}
