// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviacAINPC.h"
#include "BehaviacAgent.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

ABehaviacAINPC::ABehaviacAINPC()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Behaviac Agent Component
	BehaviacAgent = CreateDefaultSubobject<UBehaviacAgentComponent>(TEXT("BehaviacAgent"));
	// Disable auto-tick: BehaviacAINPC manually calls TickBehaviorTree() in its own Tick()
	// to have full control over tick ordering. Leaving bAutoTick=true would double-tick every frame.
	BehaviacAgent->bAutoTick = false;

	// Default properties
	DetectionRadius = 1000.0f;
	WalkSpeed = 200.0f;
	RunSpeed = 400.0f;
	AttackRange = 150.0f;
	CombatRange = 200.0f;
	CurrentPatrolIndex = 0;
	TickCounter = 0;
	DebugTimer = 0.0f;
	bHasLastKnownPos = false;
	LookAroundYaw = 0.0f;
	LookAroundDir = 1;
	GuardRadius = 1500.0f;  // Default: 1500 units from spawn

	// Set up AI Controller
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABehaviacAINPC::BeginPlay()
{
	Super::BeginPlay();

	// Check AI Controller first
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ BehaviacAINPC [%s]: NO AI CONTROLLER! Check Auto Possess AI setting!"), *GetName());
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("✅ BehaviacAINPC [%s]: AI Controller OK"), *GetName());

	// Record guard ground center at spawn location
	GuardCenter = GetActorLocation();

	// Initialize patrol points (you can set these in Blueprint or level)
	FVector StartLocation = GetActorLocation();
	PatrolPoints.Add(StartLocation + FVector(500, 0, 0));
	PatrolPoints.Add(StartLocation + FVector(500, 500, 0));
	PatrolPoints.Add(StartLocation + FVector(0, 500, 0));
	PatrolPoints.Add(StartLocation);
	
	UE_LOG(LogTemp, Warning, TEXT("🎯 BehaviacAINPC [%s]: Patrol points set, starting at: %s"), *GetName(), *StartLocation.ToString());

	// Register methods with Behaviac BEFORE loading the tree
	if (BehaviacAgent)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔧 Registering Behaviac methods..."));
		
		// Register FindPlayer method
		BehaviacAgent->RegisterMethodHandler(TEXT("FindPlayer"), [this]() -> EBehaviacStatus {
			UE_LOG(LogTemp, Warning, TEXT("▶️ [ACTION] FindPlayer - STARTED"));
			EBehaviacStatus Result = FindPlayer();
			UE_LOG(LogTemp, Warning, TEXT("◀️ [ACTION] FindPlayer - FINISHED (result: %s)"), 
				Result == EBehaviacStatus::Success ? TEXT("Success") : TEXT("Failure"));
			return Result;
		});

		// Register Patrol method
		BehaviacAgent->RegisterMethodHandler(TEXT("Patrol"), [this]() -> EBehaviacStatus {
			UE_LOG(LogTemp, Warning, TEXT("▶️ [ACTION] Patrol - STARTED"));
			EBehaviacStatus Result = Patrol();
			UE_LOG(LogTemp, Warning, TEXT("◀️ [ACTION] Patrol - FINISHED (result: %s)"), 
				Result == EBehaviacStatus::Success ? TEXT("Success") : TEXT("Failure"));
			return Result;
		});

		// Register MoveToTarget method
		BehaviacAgent->RegisterMethodHandler(TEXT("MoveToTarget"), [this]() -> EBehaviacStatus {
			UE_LOG(LogTemp, Warning, TEXT("▶️ [ACTION] MoveToTarget - STARTED"));
			EBehaviacStatus Result = MoveToTarget();
			UE_LOG(LogTemp, Warning, TEXT("◀️ [ACTION] MoveToTarget - FINISHED (result: %s)"), 
				Result == EBehaviacStatus::Success ? TEXT("Success") : TEXT("Failure"));
			return Result;
		});

		// ── New BT_PatrolGuard methods ─────────────────────────────────
		BehaviacAgent->RegisterMethodHandler(TEXT("UpdateAIState"),    [this]() { return UpdateAIState(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("ChasePlayer"),      [this]() { return ChasePlayer(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("AttackPlayer"),     [this]() { return AttackPlayer(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("StopMovement"),     [this]() { return StopMovement(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("FaceTarget"),       [this]() { return FaceTarget(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("SetWalkSpeed"),     [this]() { return SetWalkSpeed(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("SetRunSpeed"),      [this]() { return SetRunSpeed(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("MoveToLastKnownPos"), [this]() { return MoveToLastKnownPos(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("LookAround"),       [this]() { return LookAround(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("ClearLastKnownPos"), [this]() { return ClearLastKnownPos(); });
		BehaviacAgent->RegisterMethodHandler(TEXT("ReturnToPost"),      [this]() { return ReturnToPost(); });
		
		UE_LOG(LogTemp, Warning, TEXT("✅ Registered 3 methods: FindPlayer, Patrol, MoveToTarget"));
	}

	// Load behavior tree if assigned
	if (BehaviacAgent && BehaviorTree)
	{
		// Set initial properties
		BehaviacAgent->SetFloatProperty(TEXT("DetectionRadius"), DetectionRadius);
		BehaviacAgent->SetFloatProperty(TEXT("WalkSpeed"), WalkSpeed);
		BehaviacAgent->SetFloatProperty(TEXT("RunSpeed"), RunSpeed);
		BehaviacAgent->SetPropertyValue(TEXT("State"), TEXT("Idle"));
		BehaviacAgent->SetPropertyValue(TEXT("HasTarget"), TEXT("false"));
		BehaviacAgent->SetPropertyValue(TEXT("AIState"), TEXT("Patrol"));

		// Load and start behavior tree
		bool bLoaded = BehaviacAgent->LoadBehaviorTree(BehaviorTree);
		
		if (bLoaded)
		{
			UE_LOG(LogTemp, Warning, TEXT("🌳 BehaviacAINPC [%s]: Behavior tree loaded successfully!"), *GetName());
			UE_LOG(LogTemp, Warning, TEXT("🎬 Behavior tree will start ticking every frame..."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("❌ BehaviacAINPC [%s]: Failed to load behavior tree!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ BehaviacAINPC [%s]: No behavior tree assigned! (Agent=%d, Tree=%d)"), 
			*GetName(), BehaviacAgent != nullptr, BehaviorTree != nullptr);
	}
}

void ABehaviacAINPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug: Log NPC status every 5 seconds
	DebugTimer += DeltaTime;
	if (DebugTimer >= 5.0f)
	{
		FVector Velocity = GetVelocity();
		UE_LOG(LogTemp, Log, TEXT("🐶 NPC [%s] Status: Pos=%s, Vel=%s, Speed=%.1f"), 
			*GetName(), 
			*GetActorLocation().ToString(), 
			*Velocity.ToString(),
			Velocity.Size());
		DebugTimer = 0.0f;
	}

	// Tick the behavior tree
	if (BehaviacAgent)
	{
		TickCounter++;
		
		EBehaviacStatus Status = BehaviacAgent->TickBehaviorTree();
		
		// Log every 120 ticks (every 2 seconds at 60fps) to see if tree is running
		if (TickCounter % 120 == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("⏱️ Behavior tree tick #%d returned: %s"), 
				TickCounter,
				Status == EBehaviacStatus::Success ? TEXT("Success") :
				Status == EBehaviacStatus::Failure ? TEXT("Failure") :
				Status == EBehaviacStatus::Running ? TEXT("Running") :
				TEXT("Invalid"));
		}
	}
}

// AI Actions - These are called by the behavior tree!

EBehaviacStatus ABehaviacAINPC::FindPlayer()
{
	// Find player pawn within detection radius
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (!PlayerPawn)
	{
		TargetPlayer = nullptr;
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("HasTarget"), TEXT("false"));
		}
		return EBehaviacStatus::Failure;
	}

	float Distance = FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation());
	
	if (Distance <= DetectionRadius)
	{
		TargetPlayer = PlayerPawn;
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("HasTarget"), TEXT("true"));
		}
		
		return EBehaviacStatus::Success;
	}

	// Player exists but out of range
	TargetPlayer = nullptr;
	if (BehaviacAgent)
	{
		BehaviacAgent->SetPropertyValue(TEXT("HasTarget"), TEXT("false"));
	}
	return EBehaviacStatus::Failure;
}

EBehaviacStatus ABehaviacAINPC::MoveToTarget()
{
	if (!TargetPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ MoveToTarget: No target!"));
		return EBehaviacStatus::Failure;
	}

	// Use AI controller to move
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ MoveToTarget: No AI Controller!"));
		return EBehaviacStatus::Failure;
	}

	// Set speed based on distance
	float Distance = FVector::Distance(GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Distance > DetectionRadius * 0.5f)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		UE_LOG(LogTemp, Log, TEXT("🏃 MoveToTarget: RUNNING to player (distance: %.1f)"), Distance);
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		UE_LOG(LogTemp, Log, TEXT("🚶 MoveToTarget: Walking to player (distance: %.1f)"), Distance);
	}

	// Move to target
	EPathFollowingRequestResult::Type Result = AIController->MoveToActor(TargetPlayer, 50.0f);
	
	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("State"), TEXT("Chase"));
		}
		return EBehaviacStatus::Success;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("⚠️ MoveToTarget: Movement request failed! (No NavMesh?)"));
	return EBehaviacStatus::Failure;
}

EBehaviacStatus ABehaviacAINPC::Patrol()
{
	if (PatrolPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ Patrol: No patrol points!"));
		return EBehaviacStatus::Failure;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Patrol: No AI Controller!"));
		return EBehaviacStatus::Failure;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
	float Distance = FVector::Distance(GetActorLocation(), TargetPoint);

	// Reached current waypoint — advance index and issue move to the next one
	if (Distance < 100.0f)
	{
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
		TargetPoint = PatrolPoints[CurrentPatrolIndex];
		UE_LOG(LogTemp, Log, TEXT("✅ Patrol: Reached waypoint, moving to point %d at %s"),
			CurrentPatrolIndex, *TargetPoint.ToString());
		AIController->MoveToLocation(TargetPoint, 50.0f);
		return EBehaviacStatus::Running;
	}

	// If the controller is already navigating, don't interrupt it — just stay Running
	UPathFollowingComponent* PF = AIController->GetPathFollowingComponent();
	if (PF && PF->GetStatus() == EPathFollowingStatus::Moving)
	{
		return EBehaviacStatus::Running;
	}

	// Not moving (idle, stopped after chase/return, or first call) — issue move
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(TargetPoint, 50.0f);
	if (Result == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ Patrol: MoveToLocation failed (no NavMesh?)"));
		return EBehaviacStatus::Failure;
	}
	UE_LOG(LogTemp, Log, TEXT("🚶 Patrol: Issued move to point %d at %s"), CurrentPatrolIndex, *TargetPoint.ToString());
	return EBehaviacStatus::Running;
}

void ABehaviacAINPC::Idle()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->StopMovement();
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("State"), TEXT("Idle"));
		}
		
		UE_LOG(LogTemp, Verbose, TEXT("BehaviacAINPC: Idle"));
	}
}

bool ABehaviacAINPC::IsPlayerInRange()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
		return false;

	float Distance = FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation());
	return Distance <= DetectionRadius;
}

void ABehaviacAINPC::SetBehaviacProperty(const FString& Key, const FString& Value)
{
	if (BehaviacAgent)
	{
		BehaviacAgent->SetPropertyValue(Key, Value);
	}
}

FString ABehaviacAINPC::GetBehaviacProperty(const FString& Key)
{
	if (BehaviacAgent)
	{
		return BehaviacAgent->GetPropertyValue(Key);
	}
	return FString();
}

// ============================================================
// BT_PatrolGuard implementations
// ============================================================

EBehaviacStatus ABehaviacAINPC::UpdateAIState()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FString NewState = TEXT("Patrol");
	float DistFromPost = FVector::Distance(GetActorLocation(), GuardCenter);

	if (PlayerPawn)
	{
		float DistToPlayer = FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation());
		float PlayerDistFromPost = FVector::Distance(PlayerPawn->GetActorLocation(), GuardCenter);

		// Line-of-sight check — raycast from eye height, ignore self
		bool bCanSee = false;
		if (DistToPlayer <= DetectionRadius)
		{
			FVector EyeLocation = GetActorLocation() + FVector(0, 0, 60.f);
			FVector PlayerCenter = PlayerPawn->GetActorLocation() + FVector(0, 0, 60.f);
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(PlayerPawn);
			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				HitResult, EyeLocation, PlayerCenter, ECC_Visibility, Params);
			bCanSee = !bBlocked;
		}

		if (bCanSee && DistToPlayer <= AttackRange)
		{
			TargetPlayer = PlayerPawn;
			bHasLastKnownPos = true;
			LastKnownPlayerPos = PlayerPawn->GetActorLocation();
			NewState = TEXT("Combat");
		}
		else if (bCanSee && PlayerDistFromPost <= GuardRadius)
		{
			TargetPlayer = PlayerPawn;
			bHasLastKnownPos = true;
			LastKnownPlayerPos = PlayerPawn->GetActorLocation();
			NewState = TEXT("Chase");
		}
		else if (bCanSee && PlayerDistFromPost > GuardRadius)
		{
			// Spotted but outside guard ground — don't chase, return to post
			TargetPlayer = nullptr;
			bHasLastKnownPos = false;
			LastKnownPlayerPos = FVector::ZeroVector;
			NewState = TEXT("ReturnToPost");
			UE_LOG(LogTemp, Warning, TEXT("🛑 Player outside guard ground, returning to post"));
		}
		else
		{
			// Cannot see player (out of range or blocked)
			if (bHasLastKnownPos || TargetPlayer != nullptr)
			{
				// Was chasing — clear target and return to post
				TargetPlayer = nullptr;
				bHasLastKnownPos = false;
				LastKnownPlayerPos = FVector::ZeroVector;
				NewState = TEXT("ReturnToPost");
			}
			else
			{
				// No player involvement — let the Patrol branch handle its own movement
				// Only force ReturnToPost if NPC somehow wandered very far (>GuardRadius) from post
				NewState = (DistFromPost > GuardRadius) ? TEXT("ReturnToPost") : TEXT("Patrol");
			}
		}
	}
	else
	{
		// No player in world
		TargetPlayer = nullptr;
		bHasLastKnownPos = false;
		NewState = (DistFromPost > GuardRadius) ? TEXT("ReturnToPost") : TEXT("Patrol");
	}

	if (BehaviacAgent)
	{
		FString OldState = BehaviacAgent->GetPropertyValue(TEXT("AIState"));
		if (OldState != NewState)
		{
			UE_LOG(LogTemp, Warning, TEXT("🔄 AIState: %s → %s"), *OldState, *NewState);
			BehaviacAgent->SetPropertyValue(TEXT("AIState"), NewState);
		}
	}

	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::ChasePlayer()
{
	// Bail out if state has changed — lets SelectorLoop re-evaluate
	if (BehaviacAgent && BehaviacAgent->GetPropertyValue(TEXT("AIState")) != TEXT("Chase"))
	{
		UE_LOG(LogTemp, Warning, TEXT("🏃 ChasePlayer: AIState no longer Chase, stopping"));
		AAIController* AIC = Cast<AAIController>(GetController());
		if (AIC) AIC->StopMovement();
		return EBehaviacStatus::Failure;
	}

	if (!TargetPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("🏃 ChasePlayer: No target"));
		return EBehaviacStatus::Failure;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) return EBehaviacStatus::Failure;

	float Dist = FVector::Distance(GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Dist <= AttackRange)
	{
		UE_LOG(LogTemp, Log, TEXT("🏃 ChasePlayer: Reached attack range"));
		return EBehaviacStatus::Success;
	}

	AIController->MoveToActor(TargetPlayer, AttackRange * 0.8f);
	UE_LOG(LogTemp, Log, TEXT("🏃 ChasePlayer: Sprinting (dist=%.0f)"), Dist);
	return EBehaviacStatus::Running;
}

EBehaviacStatus ABehaviacAINPC::AttackPlayer()
{
	// Bail out if state changed
	if (BehaviacAgent && BehaviacAgent->GetPropertyValue(TEXT("AIState")) != TEXT("Combat"))
	{
		return EBehaviacStatus::Failure;
	}

	if (!TargetPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚔️ AttackPlayer: No target!"));
		return EBehaviacStatus::Failure;
	}

	float Dist = FVector::Distance(GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Dist > CombatRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚔️ AttackPlayer: Target out of combat range (%.0f > %.0f)"), Dist, CombatRange);
		return EBehaviacStatus::Failure;
	}

	// TODO: apply damage via damage system
	UE_LOG(LogTemp, Warning, TEXT("⚔️ AttackPlayer: HIT! (dist=%.0f)"), Dist);
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::StopMovement()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->StopMovement();
	}
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::FaceTarget()
{
	if (!TargetPlayer) return EBehaviacStatus::Failure;

	FVector Dir = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FRotator LookAt = Dir.Rotation();
	LookAt.Pitch = 0.f;
	LookAt.Roll  = 0.f;
	SetActorRotation(LookAt);
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::SetWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::SetRunSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::MoveToLastKnownPos()
{
	if (!bHasLastKnownPos)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔍 MoveToLastKnownPos: No last known position"));
		return EBehaviacStatus::Failure;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) return EBehaviacStatus::Failure;

	float Dist = FVector::Distance(GetActorLocation(), LastKnownPlayerPos);
	if (Dist < 100.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("🔍 MoveToLastKnownPos: Arrived at last known position"));
		return EBehaviacStatus::Success;
	}

	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(LastKnownPlayerPos, 80.0f);
	if (Result == EPathFollowingRequestResult::RequestSuccessful ||
		Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("🔍 MoveToLastKnownPos: Moving (dist=%.0f)"), Dist);
		return EBehaviacStatus::Success;
	}

	return EBehaviacStatus::Failure;
}

EBehaviacStatus ABehaviacAINPC::LookAround()
{
	// Rotate 45° in the current direction, then flip for next call
	FRotator Current = GetActorRotation();
	Current.Yaw += 45.0f * LookAroundDir;
	LookAroundDir = -LookAroundDir;  // Alternate left/right each call
	SetActorRotation(Current);

	UE_LOG(LogTemp, Log, TEXT("👀 LookAround: Yaw=%.1f"), Current.Yaw);
	return EBehaviacStatus::Success;
}

EBehaviacStatus ABehaviacAINPC::ReturnToPost()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) return EBehaviacStatus::Failure;

	float Dist = FVector::Distance(GetActorLocation(), GuardCenter);
	if (Dist < 100.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("🏠 ReturnToPost: Back at guard post"));
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("AIState"), TEXT("Patrol"));
		}
		return EBehaviacStatus::Success;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(GuardCenter, 80.0f);
	if (Result == EPathFollowingRequestResult::RequestSuccessful ||
		Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("🏠 ReturnToPost: Heading back (dist=%.0f)"), Dist);
		return EBehaviacStatus::Running;
	}

	return EBehaviacStatus::Failure;
}

EBehaviacStatus ABehaviacAINPC::ClearLastKnownPos()
{
	bHasLastKnownPos = false;
	LastKnownPlayerPos = FVector::ZeroVector;
	TargetPlayer = nullptr;

	if (BehaviacAgent)
	{
		BehaviacAgent->SetPropertyValue(TEXT("AIState"), TEXT("Patrol"));
	}

	UE_LOG(LogTemp, Warning, TEXT("🔍 ClearLastKnownPos: Investigation complete, returning to patrol"));
	return EBehaviacStatus::Success;
}

