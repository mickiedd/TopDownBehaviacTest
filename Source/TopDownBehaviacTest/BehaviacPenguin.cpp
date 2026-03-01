// BehaviacPenguin — see BehaviacPenguin.h

#include "BehaviacPenguin.h"
#include "BehaviacTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

// ============================================================
// Constructor
// ============================================================

ABehaviacPenguin::ABehaviacPenguin()
{
	PrimaryActorTick.bCanEverTick = true;

	PuertsComp = CreateDefaultSubobject<UPuertsNPCComponent>(TEXT("PuertsComp"));
	PuertsComp->ScriptModule = TEXT("penguin_logic");
}

// ============================================================
// BeginPlay — register handlers BEFORE Super (base loads BT)
// ============================================================

void ABehaviacPenguin::BeginPlay()
{
	if (!BehaviacAgent)
		BehaviacAgent = FindComponentByClass<UBehaviacAgentComponent>();

	if (BehaviacAgent)
	{
		BehaviacAgent->RegisterMethodHandler(TEXT("RollMood"),          [this]() { return RollMood();          });
		BehaviacAgent->RegisterMethodHandler(TEXT("PickWanderTarget"),   [this]() { return PickWanderTarget();   });
		BehaviacAgent->RegisterMethodHandler(TEXT("MoveToWanderTarget"), [this]() { return MoveToWanderTarget(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("StopMovement"),       [this]() { return StopMovement();       });
		BehaviacAgent->RegisterMethodHandler(TEXT("LookAround"),         [this]() { return LookAround();         });
		BehaviacAgent->RegisterMethodHandler(TEXT("SetSleepySpeed"),     [this]() { return SetSleepySpeed();     });
		BehaviacAgent->RegisterMethodHandler(TEXT("SetWanderSpeed"),     [this]() { return SetWanderSpeed();     });
		BehaviacAgent->RegisterMethodHandler(TEXT("SetExcitedSpeed"),    [this]() { return SetExcitedSpeed();    });
		BehaviacAgent->RegisterMethodHandler(TEXT("MaybeSpin"),          [this]() { return MaybeSpin();          });
		BehaviacAgent->RegisterMethodHandler(TEXT("SpinAround"),         [this]() { return SpinAround();         });
		BehaviacAgent->RegisterMethodHandler(TEXT("ExcitedJump"),        [this]() { return ExcitedJump();        });

		BehaviacAgent->SetBoolProperty  (TEXT("IsMoving"),     false);
		BehaviacAgent->SetFloatProperty (TEXT("WanderRadius"), WanderRadius);
		BehaviacAgent->SetFloatProperty (TEXT("WanderSpeed"),  WanderSpeed);
		BehaviacAgent->SetFloatProperty (TEXT("MoodRoll"),     0.f);
	}

	// Base class loads the behavior tree
	Super::BeginPlay();

	BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s initialized, WanderRadius=%.0f"), *GetName(), WanderRadius);
}

// ============================================================
// Tick
// ============================================================

void ABehaviacPenguin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!BehaviacAgent) return;

	UpdateBehaviacProperties();

	// Smooth LookAround interpolation (C++ fallback path only)
	if (bLookingAround)
	{
		FRotator Current = GetActorRotation();
		FRotator Target  = FRotator(Current.Pitch, LookAroundTargetYaw, Current.Roll);
		FRotator NewRot  = FMath::RInterpConstantTo(Current, Target, DeltaTime, TurnInterpSpeed);
		SetActorRotation(NewRot);

		if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, LookAroundTargetYaw)) < 2.f)
		{
			SetActorRotation(Target);
			bLookingAround      = false;
			bLookAroundComplete = true;
		}
	}

	TickCounter++;
	BehaviacAgent->TickBehaviorTree();
}

// ============================================================
// JS dispatch bridge
// ============================================================

EBehaviacStatus ABehaviacPenguin::DispatchOrRun(const FString& ActionName, TFunction<EBehaviacStatus()> CppImpl)
{
	if (PuertsComp)
	{
		int32 Result = PuertsComp->DispatchBTAction(ActionName);
		if (Result != INT32_MIN)
		{
			switch (Result)
			{
				case 0:  return EBehaviacStatus::Running;
				case 1:  return EBehaviacStatus::Success;
				case 2:  return EBehaviacStatus::Failure;
				default: return EBehaviacStatus::Success;
			}
		}
	}
	return CppImpl();
}

// ============================================================
// BT action stubs — all route through DispatchOrRun
// ============================================================

EBehaviacStatus ABehaviacPenguin::RollMood()
{
	return DispatchOrRun(TEXT("RollMood"), [this]() { return CPP_RollMood(); });
}

EBehaviacStatus ABehaviacPenguin::PickWanderTarget()
{
	return DispatchOrRun(TEXT("PickWanderTarget"), [this]() -> EBehaviacStatus
	{
		// C++ fallback: pick random point, store via base class SetNavTarget
		float Angle    = FMath::FRandRange(0.f, 2.f * PI);
		float Distance = FMath::FRandRange(WanderRadius * 0.3f, WanderRadius);
		FVector Spawn  = GetActorLocation(); // approximate — JS tracks true spawn
		SetNavTarget(
			Spawn.X + FMath::Cos(Angle) * Distance,
			Spawn.Y + FMath::Sin(Angle) * Distance);
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::MoveToWanderTarget()
{
	return DispatchOrRun(TEXT("MoveToWanderTarget"), [this]() -> EBehaviacStatus
	{
		int32 R = NavMoveToTarget(WanderAcceptanceRadius);
		if (R == 0) return EBehaviacStatus::Running;
		if (R == 1) return EBehaviacStatus::Success;
		return EBehaviacStatus::Failure;
	});
}

EBehaviacStatus ABehaviacPenguin::StopMovement()
{
	return DispatchOrRun(TEXT("StopMovement"), [this]() -> EBehaviacStatus
	{
		NavStop();
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::LookAround()
{
	return DispatchOrRun(TEXT("LookAround"), [this]() { return CPP_LookAround(); });
}

EBehaviacStatus ABehaviacPenguin::SetSleepySpeed()
{
	return DispatchOrRun(TEXT("SetSleepySpeed"), [this]() -> EBehaviacStatus {
		SetMaxSpeed(WanderSpeed * 0.6f);
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::SetWanderSpeed()
{
	return DispatchOrRun(TEXT("SetWanderSpeed"), [this]() -> EBehaviacStatus {
		SetMaxSpeed(WanderSpeed);
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::SetExcitedSpeed()
{
	return DispatchOrRun(TEXT("SetExcitedSpeed"), [this]() -> EBehaviacStatus {
		SetMaxSpeed(WanderSpeed * 2.5f);
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::MaybeSpin()
{
	return DispatchOrRun(TEXT("MaybeSpin"), [this]() -> EBehaviacStatus {
		if (FMath::FRand() < 0.4f)
		{
			FRotator R = GetActorRotation();
			R.Yaw = FRotator::ClampAxis(R.Yaw + FMath::FRandRange(90.f, 270.f));
			SetActorRotation(R);
		}
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::SpinAround()
{
	return DispatchOrRun(TEXT("SpinAround"), [this]() -> EBehaviacStatus {
		FRotator R = GetActorRotation();
		R.Yaw = FRotator::ClampAxis(R.Yaw + 180.f);
		SetActorRotation(R);
		return EBehaviacStatus::Success;
	});
}

EBehaviacStatus ABehaviacPenguin::ExcitedJump()
{
	return DispatchOrRun(TEXT("ExcitedJump"), [this]() -> EBehaviacStatus {
		LaunchCharacter(FVector(0.f, 0.f, 420.f), false, true);
		return EBehaviacStatus::Success;
	});
}

// ============================================================
// C++ fallback implementations
// ============================================================

void ABehaviacPenguin::UpdateBehaviacProperties()
{
	BehaviacAgent->SetBoolProperty(TEXT("IsMoving"), GetVelocity().SizeSquared2D() > 25.f);
}

EBehaviacStatus ABehaviacPenguin::CPP_RollMood()
{
	MoodRoll = FMath::FRand();
	BehaviacAgent->SetFloatProperty(TEXT("MoodRoll"), MoodRoll);
	BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s: CPP_RollMood → %.2f"), *GetName(), MoodRoll);
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacPenguin::CPP_LookAround()
{
	if (bLookAroundComplete)
	{
		bLookAroundComplete = false;
		return EBehaviacStatus::Success;
	}
	if (!bLookingAround)
	{
		float Delta         = FMath::FRandRange(30.f, 330.f);
		LookAroundTargetYaw = FRotator::ClampAxis(GetActorRotation().Yaw + Delta);
		bLookingAround      = true;
		BEHAVIAC_VLOG(TEXT("[BehaviacPenguin] %s: CPP_LookAround → %.1f°"), *GetName(), LookAroundTargetYaw);
	}
	return EBehaviacStatus::Running;
}
