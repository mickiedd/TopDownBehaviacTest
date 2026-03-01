// BehaviacPenguin — see BehaviacPenguin.h for behavior description.

#include "BehaviacPenguin.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviacAgent.h"
#include "BehaviacTypes.h"

// ============================================================
// Constructor
// ============================================================

ABehaviacPenguin::ABehaviacPenguin()
{
	PrimaryActorTick.bCanEverTick = true;
}

// ============================================================
// BeginPlay — register handlers BEFORE Super (base loads BT)
// ============================================================

void ABehaviacPenguin::BeginPlay()
{
	SpawnLocation = GetActorLocation();

	// Ensure BehaviacAgent is available (base constructor creates it)
	if (!BehaviacAgent)
	{
		BehaviacAgent = FindComponentByClass<UBehaviacAgentComponent>();
	}

	if (BehaviacAgent)
	{
		// Register BT method handlers before base loads the tree
		BehaviacAgent->RegisterMethodHandler(TEXT("PickWanderTarget"),   [this]() { return PickWanderTarget();   });
		BehaviacAgent->RegisterMethodHandler(TEXT("MoveToWanderTarget"), [this]() { return MoveToWanderTarget(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("StopMovement"),       [this]() { return StopMovement();       });
		BehaviacAgent->RegisterMethodHandler(TEXT("LookAround"),         [this]() { return LookAround();         });

		BehaviacAgent->SetBoolProperty  (TEXT("IsMoving"),     false);
		BehaviacAgent->SetFloatProperty (TEXT("WanderRadius"), WanderRadius);
		BehaviacAgent->SetFloatProperty (TEXT("WanderSpeed"),  WanderSpeed);
	}

	// Base class loads the behavior tree
	Super::BeginPlay();

	BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s initialized at %s, WanderRadius=%.0f"),
		*GetName(), *SpawnLocation.ToString(), WanderRadius);
}

// ============================================================
// Tick
// ============================================================

void ABehaviacPenguin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!BehaviacAgent) return;

	UpdateBehaviacProperties();

	// Smooth LookAround interpolation
	if (bLookingAround)
	{
		FRotator Current = GetActorRotation();
		FRotator Target  = FRotator(Current.Pitch, LookAroundTargetYaw, Current.Roll);
		FRotator NewRot  = FMath::RInterpConstantTo(Current, Target, DeltaTime, TurnInterpSpeed);
		SetActorRotation(NewRot);

		float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, LookAroundTargetYaw));
		if (YawDelta < 2.f)
		{
			SetActorRotation(Target);
			bLookingAround      = false;
			bLookAroundComplete = true;
		}
	}

	TickCounter++;
	EBehaviacStatus Status = BehaviacAgent->TickBehaviorTree();

	if (TickCounter % 300 == 0)
	{
		BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s tick#%d → %s | Pos=%s"),
			*GetName(), TickCounter,
			Status == EBehaviacStatus::Running ? TEXT("Running") :
			Status == EBehaviacStatus::Success ? TEXT("Success") : TEXT("Failure/Other"),
			*GetActorLocation().ToString());
	}
}

// ============================================================
// Private helpers
// ============================================================

void ABehaviacPenguin::UpdateBehaviacProperties()
{
	bool bMoving = GetVelocity().SizeSquared2D() > 25.f;
	BehaviacAgent->SetBoolProperty(TEXT("IsMoving"), bMoving);
}

// ============================================================
// BT Actions
// ============================================================

EBehaviacStatus ABehaviacPenguin::PickWanderTarget()
{
	float Angle    = FMath::FRandRange(0.f, 2.f * PI);
	float Distance = FMath::FRandRange(WanderRadius * 0.3f, WanderRadius);

	WanderTarget = SpawnLocation + FVector(
		FMath::Cos(Angle) * Distance,
		FMath::Sin(Angle) * Distance,
		0.f);

	bHasWanderTarget = true;

	BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s: new wander target %s (dist=%.0f)"),
		*GetName(), *WanderTarget.ToString(), Distance);

	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacPenguin::MoveToWanderTarget()
{
	if (!bHasWanderTarget) return EBehaviacStatus::Failure;

	AAIController* AIC = GetController<AAIController>();
	if (!AIC) return EBehaviacStatus::Failure;

	float Dist2D = FVector::Dist2D(GetActorLocation(), WanderTarget);
	if (Dist2D <= WanderAcceptanceRadius)
	{
		AIC->StopMovement();
		return EBehaviacStatus::Success;
	}

	GetCharacterMovement()->MaxWalkSpeed = WanderSpeed;

	EPathFollowingRequestResult::Type Result = AIC->MoveToLocation(
		WanderTarget,
		WanderAcceptanceRadius * 0.8f,
		true, true, true, false);

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)  return EBehaviacStatus::Success;
	if (Result == EPathFollowingRequestResult::RequestSuccessful) return EBehaviacStatus::Running;

	BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s: MoveToLocation failed — skipping"), *GetName());
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacPenguin::StopMovement()
{
	if (AAIController* AIC = GetController<AAIController>())
		AIC->StopMovement();

	bHasWanderTarget = false;
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacPenguin::LookAround()
{
	if (bLookAroundComplete)
	{
		bLookAroundComplete = false;
		return EBehaviacStatus::Success;
	}

	if (!bLookingAround)
	{
		float CurrentYaw    = GetActorRotation().Yaw;
		float Delta         = FMath::FRandRange(30.f, 330.f);
		LookAroundTargetYaw = FRotator::ClampAxis(CurrentYaw + Delta);
		bLookingAround      = true;

		BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s: LookAround → yaw=%.1f (delta=%.1f°)"),
			*GetName(), LookAroundTargetYaw, Delta);
	}

	return EBehaviacStatus::Running;
}
