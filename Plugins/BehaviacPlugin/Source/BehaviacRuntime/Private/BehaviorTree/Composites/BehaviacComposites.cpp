// Behaviac UE5 Plugin
// Licensed under the BSD 3-Clause License.

#include "BehaviorTree/Composites/BehaviacComposites.h"
#include "BehaviacAgent.h"

// ===================================================================
// SELECTOR
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSelector::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSelectorTask>(Outer);
}

void UBehaviacSelector::LoadFromProperties(int32 Version, const FString& InAgentType, const TArray<FBehaviacProperty>& Properties)
{
	Super::LoadFromProperties(Version, InAgentType, Properties);
}

bool UBehaviacSelector::Evaluate(UBehaviacAgentComponent* Agent) const
{
	for (UBehaviacBehaviorNode* Child : Children)
	{
		const UBehaviacSelector* ChildSelector = Cast<UBehaviacSelector>(Child);
		if (ChildSelector && ChildSelector->Evaluate(Agent))
		{
			return true;
		}
	}
	return false;
}

bool UBehaviacSelector::CheckIfInterrupted(UBehaviacAgentComponent* Agent) const
{
	return false;
}

bool UBehaviacSelectorTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	ActiveChildIndex = 0;
	return true;
}

void UBehaviacSelectorTask::OnExit(UBehaviacAgentComponent* Agent, EBehaviacStatus InStatus)
{
}

EBehaviacStatus UBehaviacSelectorTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	// If the current child returned success or is still running, propagate that
	if (ChildStatus == EBehaviacStatus::Success)
	{
		return EBehaviacStatus::Success;
	}

	// Current child failed, try next
	if (ChildStatus == EBehaviacStatus::Failure)
	{
		ActiveChildIndex++;
	}

	// Try children from current index
	while (ChildTasks.IsValidIndex(ActiveChildIndex))
	{
		EBehaviacStatus Result = ChildTasks[ActiveChildIndex]->Execute(Agent, EBehaviacStatus::Invalid);

		if (Result == EBehaviacStatus::Running)
		{
			return EBehaviacStatus::Running;
		}

		if (Result == EBehaviacStatus::Success)
		{
			return EBehaviacStatus::Success;
		}

		// Failure: move to next child
		ActiveChildIndex++;
	}

	// All children failed
	return EBehaviacStatus::Failure;
}

// ===================================================================
// SEQUENCE
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSequence::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSequenceTask>(Outer);
}

void UBehaviacSequence::LoadFromProperties(int32 Version, const FString& InAgentType, const TArray<FBehaviacProperty>& Properties)
{
	Super::LoadFromProperties(Version, InAgentType, Properties);
}

bool UBehaviacSequenceTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	ActiveChildIndex = 0;
	return true;
}

void UBehaviacSequenceTask::OnExit(UBehaviacAgentComponent* Agent, EBehaviacStatus InStatus)
{
}

EBehaviacStatus UBehaviacSequenceTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	// If current child is running, keep running
	if (ChildStatus == EBehaviacStatus::Running)
	{
		if (ChildTasks.IsValidIndex(ActiveChildIndex))
		{
			return ChildTasks[ActiveChildIndex]->Execute(Agent, EBehaviacStatus::Invalid);
		}
	}

	// If current child failed, propagate failure
	if (ChildStatus == EBehaviacStatus::Failure)
	{
		return EBehaviacStatus::Failure;
	}

	// Current child succeeded, move to next
	if (ChildStatus == EBehaviacStatus::Success)
	{
		ActiveChildIndex++;
	}

	// Try children from current index
	while (ChildTasks.IsValidIndex(ActiveChildIndex))
	{
		EBehaviacStatus Result = ChildTasks[ActiveChildIndex]->Execute(Agent, EBehaviacStatus::Invalid);

		if (Result == EBehaviacStatus::Running)
		{
			return EBehaviacStatus::Running;
		}

		if (Result == EBehaviacStatus::Failure)
		{
			return EBehaviacStatus::Failure;
		}

		// Success: move to next child
		ActiveChildIndex++;
	}

	// All children succeeded
	return EBehaviacStatus::Success;
}

// ===================================================================
// PARALLEL
// ===================================================================

UBehaviacParallel::UBehaviacParallel()
	: FailurePolicy(EBehaviacParallelPolicy::FailOnOne_SucceedOnAll)
	, ChildFinishPolicy(EBehaviacChildFinishPolicy::Once)
{
}

UBehaviacBehaviorTask* UBehaviacParallel::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacParallelTask>(Outer);
}

void UBehaviacParallel::LoadFromProperties(int32 Version, const FString& InAgentType, const TArray<FBehaviacProperty>& Properties)
{
	Super::LoadFromProperties(Version, InAgentType, Properties);

	for (const FBehaviacProperty& Prop : Properties)
	{
		if (Prop.Name == TEXT("FailurePolicy"))
		{
			if (Prop.Value == TEXT("FAIL_ON_ONE"))
				FailurePolicy = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
			else if (Prop.Value == TEXT("FAIL_ON_ALL"))
				FailurePolicy = EBehaviacParallelPolicy::FailOnAll_SucceedOnOne;
		}
		else if (Prop.Name == TEXT("ChildFinishPolicy"))
		{
			if (Prop.Value == TEXT("CHILDFINISH_LOOP"))
				ChildFinishPolicy = EBehaviacChildFinishPolicy::Loop;
			else
				ChildFinishPolicy = EBehaviacChildFinishPolicy::Once;
		}
	}
}

UBehaviacParallelTask::UBehaviacParallelTask()
{
}

void UBehaviacParallelTask::Init(UBehaviacBehaviorNode* InNode)
{
	Super::Init(InNode);
	ChildStatuses.SetNum(ChildTasks.Num());
	for (int32 i = 0; i < ChildStatuses.Num(); i++)
	{
		ChildStatuses[i] = EBehaviacStatus::Invalid;
	}
}

void UBehaviacParallelTask::Reset(UBehaviacAgentComponent* Agent)
{
	Super::Reset(Agent);
	for (int32 i = 0; i < ChildStatuses.Num(); i++)
	{
		ChildStatuses[i] = EBehaviacStatus::Invalid;
	}
}

bool UBehaviacParallelTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	return true;
}

EBehaviacStatus UBehaviacParallelTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	const UBehaviacParallel* ParallelNode = Cast<UBehaviacParallel>(Node);
	if (!ParallelNode)
	{
		return EBehaviacStatus::Failure;
	}

	int32 SuccessCount = 0;
	int32 FailCount = 0;
	int32 RunningCount = 0;

	for (int32 i = 0; i < ChildTasks.Num(); i++)
	{
		// Skip completed children if policy is Once
		if (ParallelNode->ChildFinishPolicy == EBehaviacChildFinishPolicy::Once &&
			ChildStatuses[i] != EBehaviacStatus::Invalid &&
			ChildStatuses[i] != EBehaviacStatus::Running)
		{
			if (ChildStatuses[i] == EBehaviacStatus::Success) SuccessCount++;
			else if (ChildStatuses[i] == EBehaviacStatus::Failure) FailCount++;
			continue;
		}

		EBehaviacStatus Result = ChildTasks[i]->Execute(Agent, EBehaviacStatus::Invalid);
		ChildStatuses[i] = Result;

		switch (Result)
		{
		case EBehaviacStatus::Success:	SuccessCount++; break;
		case EBehaviacStatus::Failure:	FailCount++; break;
		case EBehaviacStatus::Running:	RunningCount++; break;
		default: break;
		}
	}

	// Determine result based on policy
	switch (ParallelNode->FailurePolicy)
	{
	case EBehaviacParallelPolicy::FailOnOne_SucceedOnAll:
		if (FailCount > 0) return EBehaviacStatus::Failure;
		if (SuccessCount == ChildTasks.Num()) return EBehaviacStatus::Success;
		break;

	case EBehaviacParallelPolicy::FailOnAll_SucceedOnOne:
		if (SuccessCount > 0) return EBehaviacStatus::Success;
		if (FailCount == ChildTasks.Num()) return EBehaviacStatus::Failure;
		break;

	case EBehaviacParallelPolicy::FailOnOne_SucceedOnOne:
		if (FailCount > 0) return EBehaviacStatus::Failure;
		if (SuccessCount > 0) return EBehaviacStatus::Success;
		break;
	}

	return EBehaviacStatus::Running;
}

// ===================================================================
// IF-ELSE
// ===================================================================

UBehaviacBehaviorTask* UBehaviacIfElse::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacIfElseTask>(Outer);
}

bool UBehaviacIfElseTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	return ChildTasks.Num() >= 2;
}

EBehaviacStatus UBehaviacIfElseTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (ChildTasks.Num() < 2)
	{
		return EBehaviacStatus::Failure;
	}

	// First child is the condition
	if (ActiveChildIndex == 0)
	{
		EBehaviacStatus CondResult = ChildTasks[0]->Execute(Agent, EBehaviacStatus::Invalid);

		if (CondResult == EBehaviacStatus::Running)
		{
			return EBehaviacStatus::Running;
		}

		// Select branch: index 1 for true, index 2 for false
		ActiveChildIndex = (CondResult == EBehaviacStatus::Success) ? 1 : 2;
	}

	// Execute selected branch
	if (ChildTasks.IsValidIndex(ActiveChildIndex))
	{
		return ChildTasks[ActiveChildIndex]->Execute(Agent, ChildStatus);
	}

	return EBehaviacStatus::Failure;
}

// ===================================================================
// SELECTOR LOOP
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSelectorLoop::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSelectorLoopTask>(Outer);
}

bool UBehaviacSelectorLoopTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	ActiveChildIndex = 0;
	return true;
}

EBehaviacStatus UBehaviacSelectorLoopTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	// Re-evaluate from the beginning to check if higher priority child is valid
	for (int32 i = 0; i < ChildTasks.Num(); i++)
	{
		// If a higher-priority child can now run, interrupt current
		if (i < ActiveChildIndex)
		{
			UBehaviacBehaviorNode* ChildNode = Node ? Node->GetChild(i) : nullptr;
			if (ChildNode)
			{
				// Reset and try this child
				EBehaviacStatus Result = ChildTasks[i]->Execute(Agent, EBehaviacStatus::Invalid);
				if (Result != EBehaviacStatus::Failure)
				{
					// Interrupt current child
					if (ChildTasks.IsValidIndex(ActiveChildIndex))
					{
						ChildTasks[ActiveChildIndex]->Reset(Agent);
					}
					ActiveChildIndex = i;
					return Result;
				}
			}
		}
		else if (i == ActiveChildIndex)
		{
			EBehaviacStatus Result = ChildTasks[i]->Execute(Agent, ChildStatus);

			if (Result == EBehaviacStatus::Running)
			{
				return EBehaviacStatus::Running;
			}

			if (Result == EBehaviacStatus::Success)
			{
				return EBehaviacStatus::Success;
			}

			ActiveChildIndex++;
		}
	}

	return EBehaviacStatus::Failure;
}

// ===================================================================
// SELECTOR PROBABILITY
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSelectorProbability::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSelectorProbabilityTask>(Outer);
}

bool UBehaviacSelectorProbabilityTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	return ChildTasks.Num() > 0;
}

EBehaviacStatus UBehaviacSelectorProbabilityTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (ChildTasks.Num() == 0)
	{
		return EBehaviacStatus::Failure;
	}

	// Pick weighted random child (weights from decorator weight nodes)
	// For simplicity, use uniform random if no weights
	if (ActiveChildIndex < 0 || ActiveChildIndex >= ChildTasks.Num())
	{
		ActiveChildIndex = FMath::RandRange(0, ChildTasks.Num() - 1);
	}

	return ChildTasks[ActiveChildIndex]->Execute(Agent, ChildStatus);
}

// ===================================================================
// SELECTOR STOCHASTIC
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSelectorStochastic::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSelectorStochasticTask>(Outer);
}

UBehaviacSelectorStochasticTask::UBehaviacSelectorStochasticTask()
{
}

void UBehaviacSelectorStochasticTask::Reset(UBehaviacAgentComponent* Agent)
{
	Super::Reset(Agent);
	ShuffledOrder.Empty();
}

bool UBehaviacSelectorStochasticTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	ActiveChildIndex = 0;

	// Create shuffled order
	ShuffledOrder.SetNum(ChildTasks.Num());
	for (int32 i = 0; i < ShuffledOrder.Num(); i++)
	{
		ShuffledOrder[i] = i;
	}

	// Fisher-Yates shuffle
	for (int32 i = ShuffledOrder.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		ShuffledOrder.Swap(i, j);
	}

	return true;
}

EBehaviacStatus UBehaviacSelectorStochasticTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (ChildStatus == EBehaviacStatus::Success)
	{
		return EBehaviacStatus::Success;
	}

	if (ChildStatus == EBehaviacStatus::Failure)
	{
		ActiveChildIndex++;
	}

	while (ActiveChildIndex < ShuffledOrder.Num())
	{
		int32 ChildIdx = ShuffledOrder[ActiveChildIndex];
		if (ChildTasks.IsValidIndex(ChildIdx))
		{
			EBehaviacStatus Result = ChildTasks[ChildIdx]->Execute(Agent, EBehaviacStatus::Invalid);
			if (Result != EBehaviacStatus::Failure)
			{
				return Result;
			}
		}
		ActiveChildIndex++;
	}

	return EBehaviacStatus::Failure;
}

// ===================================================================
// SEQUENCE STOCHASTIC
// ===================================================================

UBehaviacBehaviorTask* UBehaviacSequenceStochastic::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacSequenceStochasticTask>(Outer);
}

UBehaviacSequenceStochasticTask::UBehaviacSequenceStochasticTask()
{
}

void UBehaviacSequenceStochasticTask::Reset(UBehaviacAgentComponent* Agent)
{
	Super::Reset(Agent);
	ShuffledOrder.Empty();
}

bool UBehaviacSequenceStochasticTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	ActiveChildIndex = 0;

	ShuffledOrder.SetNum(ChildTasks.Num());
	for (int32 i = 0; i < ShuffledOrder.Num(); i++)
	{
		ShuffledOrder[i] = i;
	}

	for (int32 i = ShuffledOrder.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		ShuffledOrder.Swap(i, j);
	}

	return true;
}

EBehaviacStatus UBehaviacSequenceStochasticTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (ChildStatus == EBehaviacStatus::Failure)
	{
		return EBehaviacStatus::Failure;
	}

	if (ChildStatus == EBehaviacStatus::Success)
	{
		ActiveChildIndex++;
	}

	while (ActiveChildIndex < ShuffledOrder.Num())
	{
		int32 ChildIdx = ShuffledOrder[ActiveChildIndex];
		if (ChildTasks.IsValidIndex(ChildIdx))
		{
			EBehaviacStatus Result = ChildTasks[ChildIdx]->Execute(Agent, EBehaviacStatus::Invalid);
			if (Result != EBehaviacStatus::Success)
			{
				return Result;
			}
		}
		ActiveChildIndex++;
	}

	return EBehaviacStatus::Success;
}

// ===================================================================
// REFERENCE BEHAVIOR
// ===================================================================

UBehaviacBehaviorTask* UBehaviacReferenceBehavior::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacReferenceBehaviorTask>(Outer);
}

void UBehaviacReferenceBehavior::LoadFromProperties(int32 Version, const FString& InAgentType, const TArray<FBehaviacProperty>& Properties)
{
	Super::LoadFromProperties(Version, InAgentType, Properties);

	for (const FBehaviacProperty& Prop : Properties)
	{
		if (Prop.Name == TEXT("ReferenceFilename") || Prop.Name == TEXT("ReferenceBehavior"))
		{
			ReferencedTreePath = Prop.Value;
		}
	}
}

bool UBehaviacReferenceBehaviorTask::OnEnter(UBehaviacAgentComponent* Agent)
{
	// The sub-tree is loaded via the workspace and linked during Init
	return true;
}

EBehaviacStatus UBehaviacReferenceBehaviorTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (SubTreeTask)
	{
		return SubTreeTask->Tick(Agent);
	}

	if (ChildTask)
	{
		return ChildTask->Execute(Agent, ChildStatus);
	}

	return EBehaviacStatus::Failure;
}

// ===================================================================
// WITH PRECONDITION
// ===================================================================

UBehaviacBehaviorTask* UBehaviacWithPrecondition::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacWithPreconditionTask>(Outer);
}

EBehaviacStatus UBehaviacWithPreconditionTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	if (ChildTasks.Num() < 2)
	{
		return EBehaviacStatus::Failure;
	}

	// First child: precondition
	EBehaviacStatus PrecondResult = ChildTasks[0]->Execute(Agent, EBehaviacStatus::Invalid);

	if (PrecondResult != EBehaviacStatus::Success)
	{
		return EBehaviacStatus::Failure;
	}

	// Second child: the actual action
	return ChildTasks[1]->Execute(Agent, ChildStatus);
}
