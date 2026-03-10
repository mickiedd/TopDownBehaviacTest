// BehaviacPenguin — see BehaviacPenguin.h

#include "BehaviacPenguin.h"
#include "BehaviacTypes.h"

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
// BeginPlay
// ============================================================

void ABehaviacPenguin::BeginPlay()
{
	if (!BehaviacAgent)
		BehaviacAgent = FindComponentByClass<UBehaviacAgentComponent>();

	if (BehaviacAgent)
	{
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
	BehaviacAgent->TickBehaviorTree();
}

void ABehaviacPenguin::UpdateBehaviacProperties()
{
	BehaviacAgent->SetBoolProperty(TEXT("IsMoving"), GetVelocity().SizeSquared2D() > 25.f);
}
