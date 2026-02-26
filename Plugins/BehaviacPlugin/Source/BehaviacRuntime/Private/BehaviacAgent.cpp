// Behaviac UE5 Plugin
// Licensed under the BSD 3-Clause License.

#include "BehaviacAgent.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "BehaviorTree/BehaviacBehaviorTask.h"
#include "BehaviorTree/BehaviacBehaviorNode.h"

UBehaviacAgentComponent::UBehaviacAgentComponent()
	: bAutoTick(true)
	, DefaultBehaviorTree(nullptr)
	, CurrentTreeTask(nullptr)
	, CurrentTreeAsset(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UBehaviacAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultBehaviorTree)
	{
		LoadBehaviorTree(DefaultBehaviorTree);
	}
}

void UBehaviacAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoTick && CurrentTreeTask)
	{
		TickBehaviorTree();
	}
}

void UBehaviacAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBehaviorTree();
	Super::EndPlay(EndPlayReason);
}

// --- Behavior Tree Management ---

bool UBehaviacAgentComponent::LoadBehaviorTree(UBehaviacBehaviorTree* TreeAsset)
{
	if (!TreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Behaviac] Cannot load null behavior tree"));
		return false;
	}

	StopBehaviorTree();

	CurrentTreeAsset = TreeAsset;

	UBehaviacBehaviorNode* RootNode = TreeAsset->GetRootNode();
	if (!RootNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Behaviac] Behavior tree has no root node: %s"), *TreeAsset->GetName());
		return false;
	}

	// Create the root task (BehaviorTreeTask wrapping the root node)
	CurrentTreeTask = NewObject<UBehaviacBehaviorTreeTask>(this);
	
	UE_LOG(LogTemp, Warning, TEXT("[Behaviac] 🔨 Creating tasks: RootNode=%d, NodeType=%s, ChildCount=%d"), 
		RootNode != nullptr, 
		RootNode ? *RootNode->GetName() : TEXT("NULL"),
		RootNode ? RootNode->GetChildCount() : -1);
	
	CurrentTreeTask->Init(RootNode);
	
	UE_LOG(LogTemp, Warning, TEXT("[Behaviac] 🔨 After Init: CurrentTreeTask->HasChildTask=%d"), 
		CurrentTreeTask->HasChildTask());

	UE_LOG(LogTemp, Log, TEXT("[Behaviac] Loaded behavior tree: %s"), *TreeAsset->GetName());
	return true;
}

bool UBehaviacAgentComponent::LoadBehaviorTreeByPath(const FString& RelativePath)
{
	// Try to find the asset
	FString AssetPath = FString::Printf(TEXT("/Game/BehaviacData/%s"), *RelativePath);
	UBehaviacBehaviorTree* TreeAsset = LoadObject<UBehaviacBehaviorTree>(nullptr, *AssetPath);

	if (TreeAsset)
	{
		return LoadBehaviorTree(TreeAsset);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Behaviac] Could not find behavior tree at path: %s"), *AssetPath);
	return false;
}

EBehaviacStatus UBehaviacAgentComponent::TickBehaviorTree()
{
	if (!CurrentTreeTask)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Behaviac] TickBehaviorTree: CurrentTreeTask is NULL!"));
		return EBehaviacStatus::Invalid;
	}

	EBehaviacStatus Result = CurrentTreeTask->Tick(this);
	return Result;
}

void UBehaviacAgentComponent::StopBehaviorTree()
{
	if (CurrentTreeTask)
	{
		CurrentTreeTask->Reset(this);
		CurrentTreeTask = nullptr;
	}

	CurrentTreeAsset = nullptr;
}

void UBehaviacAgentComponent::ResetBehaviorTree()
{
	if (CurrentTreeTask)
	{
		CurrentTreeTask->Reset(this);
	}
}

EBehaviacStatus UBehaviacAgentComponent::GetBehaviorTreeStatus() const
{
	if (CurrentTreeTask)
	{
		return CurrentTreeTask->GetTreeStatus();
	}
	return EBehaviacStatus::Invalid;
}

// --- Property System ---

void UBehaviacAgentComponent::SetPropertyValue(const FString& PropertyName, const FString& Value)
{
	FScopeLock Lock(&PropertyLock);

	// Strip "Self." prefix if present
	FString CleanName = PropertyName;
	if (CleanName.StartsWith(TEXT("Self.")))
	{
		CleanName = CleanName.Mid(5);
	}

	Properties.Add(CleanName, Value);
}

FString UBehaviacAgentComponent::GetPropertyValue(const FString& PropertyName) const
{
	FScopeLock Lock(&PropertyLock);

	FString CleanName = PropertyName;
	if (CleanName.StartsWith(TEXT("Self.")))
	{
		CleanName = CleanName.Mid(5);
	}

	const FString* Found = Properties.Find(CleanName);
	return Found ? *Found : FString();
}

bool UBehaviacAgentComponent::HasProperty(const FString& PropertyName) const
{
	FScopeLock Lock(&PropertyLock);

	FString CleanName = PropertyName;
	if (CleanName.StartsWith(TEXT("Self.")))
	{
		CleanName = CleanName.Mid(5);
	}

	return Properties.Contains(CleanName);
}

void UBehaviacAgentComponent::SetIntProperty(const FString& PropertyName, int32 Value)
{
	SetPropertyValue(PropertyName, FString::FromInt(Value));
}

int32 UBehaviacAgentComponent::GetIntProperty(const FString& PropertyName) const
{
	return FCString::Atoi(*GetPropertyValue(PropertyName));
}

void UBehaviacAgentComponent::SetFloatProperty(const FString& PropertyName, float Value)
{
	SetPropertyValue(PropertyName, FString::SanitizeFloat(Value));
}

float UBehaviacAgentComponent::GetFloatProperty(const FString& PropertyName) const
{
	return FCString::Atof(*GetPropertyValue(PropertyName));
}

void UBehaviacAgentComponent::SetBoolProperty(const FString& PropertyName, bool Value)
{
	SetPropertyValue(PropertyName, Value ? TEXT("true") : TEXT("false"));
}

bool UBehaviacAgentComponent::GetBoolProperty(const FString& PropertyName) const
{
	FString Val = GetPropertyValue(PropertyName);
	return Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1");
}

// --- Method System ---

EBehaviacStatus UBehaviacAgentComponent::ExecuteMethod(const FString& MethodName)
{
	UE_LOG(LogTemp, Warning, TEXT("[Behaviac] 🔍 ExecuteMethod called for: '%s'"), *MethodName);

	// First check registered C++ handlers
	if (TFunction<EBehaviacStatus()>* Handler = MethodHandlers.Find(MethodName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Behaviac] ✅ Found C++ handler for '%s', calling it..."), *MethodName);
		return (*Handler)();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Behaviac] ❌ No C++ handler found for '%s' (have %d handlers registered)"), *MethodName, MethodHandlers.Num());
		
		// Debug: List all registered handlers
		for (const auto& Pair : MethodHandlers)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Behaviac]    - Registered: '%s'"), *Pair.Key);
		}
	}

	// Try Blueprint delegate
	if (OnMethodCalled.IsBound())
	{
		EBehaviacStatus Result = EBehaviacStatus::Invalid;
		OnMethodCalled.Broadcast(MethodName, Result);
		if (Result != EBehaviacStatus::Invalid)
		{
			return Result;
		}
	}

	// Fall back to Blueprint implementable event
	EBehaviacStatus BlueprintResult = OnExecuteMethod(MethodName);
	if (BlueprintResult != EBehaviacStatus::Invalid)
	{
		return BlueprintResult;
	}

	UE_LOG(LogTemp, Verbose, TEXT("[Behaviac] No handler for method: %s"), *MethodName);
	return EBehaviacStatus::Invalid;
}

void UBehaviacAgentComponent::RegisterMethodHandler(const FString& MethodName, TFunction<EBehaviacStatus()> Handler)
{
	MethodHandlers.Add(MethodName, MoveTemp(Handler));
}

// --- Signal System ---

void UBehaviacAgentComponent::SendSignal(const FString& SignalName)
{
	ActiveSignals.Add(SignalName);
	OnSignalReceived.Broadcast(SignalName);
}

bool UBehaviacAgentComponent::IsSignalSet(const FString& SignalName) const
{
	return ActiveSignals.Contains(SignalName);
}

void UBehaviacAgentComponent::ClearSignal(const FString& SignalName)
{
	ActiveSignals.Remove(SignalName);
}

void UBehaviacAgentComponent::ClearAllSignals()
{
	ActiveSignals.Empty();
}

// --- Event System ---

void UBehaviacAgentComponent::FireEvent(const FString& EventName)
{
	PendingEvents.Add(EventName);
}

bool UBehaviacAgentComponent::HasPendingEvent(const FString& EventName) const
{
	return PendingEvents.Contains(EventName);
}

void UBehaviacAgentComponent::ConsumeEvent(const FString& EventName)
{
	PendingEvents.Remove(EventName);
}
