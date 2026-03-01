// BehaviacAnimalBase — see BehaviacAnimalBase.h for details.

#include "BehaviacAnimalBase.h"
#include "BehaviacTypes.h"

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
