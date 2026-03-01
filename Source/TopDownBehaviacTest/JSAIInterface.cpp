// Copyright Epic Games, Inc. All Rights Reserved.

#include "JSAIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"

UJSAIInterface::UJSAIInterface()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UJSAIInterface::BeginPlay()
{
    Super::BeginPlay();

    // Cache guard center (spawn location)
    if (AActor* Owner = GetOwner())
    {
        GuardCenter = Owner->GetActorLocation();
        BehaviacAgent = Owner->FindComponentByClass<UBehaviacAgentComponent>();
        if (!BehaviacAgent)
        {
            UE_LOG(LogTemp, Warning, TEXT("[JSAIInterface] No UBehaviacAgentComponent found on %s — state calls will be no-ops."), *Owner->GetName());
        }
    }
}

// ── Private helpers ───────────────────────────────────────────────────────────

AAIController* UJSAIInterface::GetAIC() const
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    return Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
}

UCharacterMovementComponent* UJSAIInterface::GetMovement() const
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    return Char ? Char->GetCharacterMovement() : nullptr;
}

APawn* UJSAIInterface::GetPlayer() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

// ── Sensor primitives ─────────────────────────────────────────────────────────

bool UJSAIInterface::CanSeePlayer() const
{
    APawn* Player = GetPlayer();
    AActor* Owner = GetOwner();
    if (!Player || !Owner) return false;

    float Dist = FVector::Distance(Owner->GetActorLocation(), Player->GetActorLocation());
    if (Dist > DetectionRadius) return false;

    FVector EyeLocation   = Owner->GetActorLocation() + FVector(0, 0, 60.f);
    FVector PlayerCenter  = Player->GetActorLocation() + FVector(0, 0, 60.f);
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    Params.AddIgnoredActor(Player);
    bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, PlayerCenter, ECC_Visibility, Params);
    return !bBlocked;
}

float UJSAIInterface::GetDistanceToPlayer() const
{
    APawn* Player = GetPlayer();
    if (!Player || !GetOwner()) return -1.f;
    return FVector::Distance(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

float UJSAIInterface::GetDistanceToTarget() const
{
    if (!TargetActor || !GetOwner()) return -1.f;
    return FVector::Distance(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
}

float UJSAIInterface::GetDistanceFromPost() const
{
    if (!GetOwner()) return -1.f;
    return FVector::Distance(GetOwner()->GetActorLocation(), GuardCenter);
}

float UJSAIInterface::GetPlayerDistanceFromPost() const
{
    APawn* Player = GetPlayer();
    if (!Player) return -1.f;
    return FVector::Distance(Player->GetActorLocation(), GuardCenter);
}

// ── Position ──────────────────────────────────────────────────────────────────

float UJSAIInterface::GetLocationX() const { return GetOwner() ? GetOwner()->GetActorLocation().X : 0.f; }
float UJSAIInterface::GetLocationY() const { return GetOwner() ? GetOwner()->GetActorLocation().Y : 0.f; }
float UJSAIInterface::GetLocationZ() const { return GetOwner() ? GetOwner()->GetActorLocation().Z : 0.f; }

float UJSAIInterface::GetSpeedXY() const
{
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        FVector V = Pawn->GetVelocity();
        return FMath::Sqrt(V.X * V.X + V.Y * V.Y);
    }
    return 0.f;
}

// ── Movement ──────────────────────────────────────────────────────────────────

void UJSAIInterface::SetSpeed(float Speed)
{
    if (UCharacterMovementComponent* MC = GetMovement())
        MC->MaxWalkSpeed = Speed;
}

void UJSAIInterface::StopMovement()
{
    if (AAIController* AIC = GetAIC()) AIC->StopMovement();
}

void UJSAIInterface::MoveToTarget()
{
    AAIController* AIC = GetAIC();
    if (AIC && TargetActor) AIC->MoveToActor(TargetActor, AttackRange * 0.8f);
}

void UJSAIInterface::MoveToPost()
{
    if (AAIController* AIC = GetAIC()) AIC->MoveToLocation(GuardCenter, 80.f);
}

bool UJSAIInterface::MoveToLastKnownPos()
{
    if (!bHasLastKnownPos) return false;
    AAIController* AIC = GetAIC();
    if (!AIC) return false;
    float Dist = FVector::Distance(GetOwner()->GetActorLocation(), LastKnownPlayerPos);
    if (Dist < 100.f) return true;
    AIC->MoveToLocation(LastKnownPlayerPos, 80.f);
    return false;
}

void UJSAIInterface::Patrol()
{
    if (PatrolPoints.Num() == 0) return;
    AAIController* AIC = GetAIC();
    if (!AIC) return;
    FVector Target = PatrolPoints[CurrentPatrolIndex % PatrolPoints.Num()];
    if (FVector::Distance(GetOwner()->GetActorLocation(), Target) < 100.f)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        Target = PatrolPoints[CurrentPatrolIndex];
    }
    AIC->MoveToLocation(Target, 80.f);
}

void UJSAIInterface::FaceTarget()
{
    if (!TargetActor || !GetOwner()) return;
    FVector Dir = (TargetActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FRotator LookAt = Dir.Rotation();
    LookAt.Pitch = 0.f;
    LookAt.Roll  = 0.f;
    GetOwner()->SetActorRotation(LookAt);
}

void UJSAIInterface::LookAround()
{
    if (!GetOwner()) return;
    FRotator Current = GetOwner()->GetActorRotation();
    Current.Yaw += 45.0f * LookAroundDir;
    LookAroundDir = -LookAroundDir;
    GetOwner()->SetActorRotation(Current);
}

// ── Goofy actions ─────────────────────────────────────────────────────────────

void UJSAIInterface::Jump()
{
    if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
        Char->Jump();
}

void UJSAIInterface::Crouch()
{
    if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
        Char->Crouch();
}

void UJSAIInterface::UnCrouch()
{
    if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
        Char->UnCrouch();
}

void UJSAIInterface::LaunchUp(float ZForce)
{
    if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
        Char->LaunchCharacter(FVector(0, 0, ZForce), false, true);
}

void UJSAIInterface::Spin(float Degrees)
{
    if (!GetOwner()) return;
    FRotator R = GetOwner()->GetActorRotation();
    R.Yaw += Degrees;
    GetOwner()->SetActorRotation(R);
}

void UJSAIInterface::Dash(float Force)
{
    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;
    FVector Dir = Char->GetActorForwardVector();
    Char->LaunchCharacter(Dir * Force, true, false);
}

void UJSAIInterface::SetSpeedRaw(float Speed)
{
    if (UCharacterMovementComponent* MC = GetMovement())
        MC->MaxWalkSpeed = Speed;
}

// ── State ─────────────────────────────────────────────────────────────────────

void UJSAIInterface::SetAIState(const FString& NewState)
{
    if (!BehaviacAgent) return;
    FString OldState = BehaviacAgent->GetPropertyValue(TEXT("AIState"));
    if (OldState != NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[JSAIInterface] AIState: %s → %s"), *OldState, *NewState);
        BehaviacAgent->SetPropertyValue(TEXT("AIState"), NewState);
    }
}

FString UJSAIInterface::GetAIState() const
{
    return BehaviacAgent ? BehaviacAgent->GetPropertyValue(TEXT("AIState")) : FString();
}

void UJSAIInterface::SetLastKnownPos()
{
    APawn* Player = GetPlayer();
    if (!Player) return;
    TargetActor = Player;
    bHasLastKnownPos = true;
    LastKnownPlayerPos = Player->GetActorLocation();
}

void UJSAIInterface::ClearLastKnownPos()
{
    bHasLastKnownPos = false;
    LastKnownPlayerPos = FVector::ZeroVector;
    TargetActor = nullptr;
}

// ── Patrol setup ──────────────────────────────────────────────────────────────

void UJSAIInterface::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}
