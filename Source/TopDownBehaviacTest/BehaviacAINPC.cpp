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
	CurrentPatrolIndex = 0;
	TickCounter = 0;
	DebugTimer = 0.0f;

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
		
		UE_LOG(LogTemp, Warning, TEXT("👀 BehaviacAINPC: Found player at distance %.1f!"), Distance);
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

	// Get current patrol point
	FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
	
	// Check if reached current patrol point
	float Distance = FVector::Distance(GetActorLocation(), TargetPoint);
	if (Distance < 100.0f)
	{
		// Move to next patrol point
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
		TargetPoint = PatrolPoints[CurrentPatrolIndex];
		
		UE_LOG(LogTemp, Log, TEXT("✅ Patrol: Reached point %d, moving to point %d"), 
			CurrentPatrolIndex - 1, CurrentPatrolIndex);
	}

	// Move to patrol point
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(TargetPoint, 50.0f);
	
	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		if (BehaviacAgent)
		{
			BehaviacAgent->SetPropertyValue(TEXT("State"), TEXT("Patrol"));
		}
		
		UE_LOG(LogTemp, Log, TEXT("🚶 Patrol: Moving to point %d at %s"), 
			CurrentPatrolIndex, *TargetPoint.ToString());
		
		return EBehaviacStatus::Success;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("⚠️ Patrol: Movement request failed! (No NavMesh?)"));
	return EBehaviacStatus::Failure;
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
