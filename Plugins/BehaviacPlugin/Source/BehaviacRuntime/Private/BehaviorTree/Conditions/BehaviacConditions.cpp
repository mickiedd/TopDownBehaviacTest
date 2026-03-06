// Behaviac UE5 Plugin
// Licensed under the BSD 3-Clause License.

#include "BehaviorTree/Conditions/BehaviacConditions.h"
#include "BehaviacAgent.h"

// ===================================================================
// CONDITION
// ===================================================================

UBehaviacBehaviorTask* UBehaviacCondition::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacConditionTask>(Outer);
}

void UBehaviacCondition::LoadFromProperties(int32 Version, const FString& InAgentType, const TArray<FBehaviacProperty>& Properties)
{
	Super::LoadFromProperties(Version, InAgentType, Properties);
	Operator = EBehaviacOperatorType::Equal;

	for (const FBehaviacProperty& Prop : Properties)
	{
		if (Prop.Name == TEXT("Opl"))
		{
			LeftOperand = Prop.Value;
		}
		else if (Prop.Name == TEXT("Opr"))
		{
			RightOperand = Prop.Value;
		}
		else if (Prop.Name == TEXT("Operator"))
		{
			if (Prop.Value == TEXT("Equal")) Operator = EBehaviacOperatorType::Equal;
			else if (Prop.Value == TEXT("NotEqual")) Operator = EBehaviacOperatorType::NotEqual;
			else if (Prop.Value == TEXT("Greater")) Operator = EBehaviacOperatorType::Greater;
			else if (Prop.Value == TEXT("Less")) Operator = EBehaviacOperatorType::Less;
			else if (Prop.Value == TEXT("GreaterEqual")) Operator = EBehaviacOperatorType::GreaterEqual;
			else if (Prop.Value == TEXT("LessEqual")) Operator = EBehaviacOperatorType::LessEqual;
		}
	}
}

EBehaviacStatus UBehaviacConditionTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	const UBehaviacCondition* CondNode = Cast<UBehaviacCondition>(Node);
	if (!CondNode || !Agent)
	{
		return EBehaviacStatus::Failure;
	}

	FString LeftStr = CondNode->LeftOperand;
	FString RightStr = CondNode->RightOperand;

	// 解析左操作数：先尝试作为属性，如果为空则尝试作为方法
	if (LeftStr.StartsWith(TEXT("Self.")))
	{
		FString PropName = LeftStr.Mid(5); // 移除 "Self." 前缀
		LeftStr = Agent->GetPropertyValue(LeftStr);

		// 如果属性不存在（返回空字符串），尝试作为方法调用
		if (LeftStr.IsEmpty())
		{
			EBehaviacStatus MethodResult = Agent->ExecuteMethod(PropName);
			// 将方法结果转换为字符串：Success=true, 其他=false
			LeftStr = (MethodResult == EBehaviacStatus::Success) ? TEXT("true") : TEXT("false");
			UE_LOG(LogBehaviac, Verbose, TEXT("[Condition] 调用方法 '%s'，结果: %s"), *PropName, *LeftStr);
		}
	}

	// 解析右操作数
	if (RightStr.StartsWith(TEXT("Self.")))
	{
		FString PropName = RightStr.Mid(5);
		RightStr = Agent->GetPropertyValue(RightStr);

		if (RightStr.IsEmpty())
		{
			EBehaviacStatus MethodResult = Agent->ExecuteMethod(PropName);
			RightStr = (MethodResult == EBehaviacStatus::Success) ? TEXT("true") : TEXT("false");
			UE_LOG(LogBehaviac, Verbose, TEXT("[Condition] 调用方法 '%s'，结果: %s"), *PropName, *RightStr);
		}
	}

	return EvaluateComparison(LeftStr, RightStr, CondNode->Operator) ?
		EBehaviacStatus::Success : EBehaviacStatus::Failure;
}

bool UBehaviacConditionTask::EvaluateComparison(const FString& Left, const FString& Right, EBehaviacOperatorType Op) const
{
	// Try numeric comparison
	if (Left.IsNumeric() && Right.IsNumeric())
	{
		double LeftVal = FCString::Atod(*Left);
		double RightVal = FCString::Atod(*Right);

		switch (Op)
		{
		case EBehaviacOperatorType::Equal:			return FMath::IsNearlyEqual(LeftVal, RightVal);
		case EBehaviacOperatorType::NotEqual:		return !FMath::IsNearlyEqual(LeftVal, RightVal);
		case EBehaviacOperatorType::Greater:			return LeftVal > RightVal;
		case EBehaviacOperatorType::Less:			return LeftVal < RightVal;
		case EBehaviacOperatorType::GreaterEqual:	return LeftVal >= RightVal;
		case EBehaviacOperatorType::LessEqual:		return LeftVal <= RightVal;
		default: return false;
		}
	}

	// String comparison
	int32 Cmp = Left.Compare(Right);
	switch (Op)
	{
	case EBehaviacOperatorType::Equal:			return Cmp == 0;
	case EBehaviacOperatorType::NotEqual:		return Cmp != 0;
	case EBehaviacOperatorType::Greater:			return Cmp > 0;
	case EBehaviacOperatorType::Less:			return Cmp < 0;
	case EBehaviacOperatorType::GreaterEqual:	return Cmp >= 0;
	case EBehaviacOperatorType::LessEqual:		return Cmp <= 0;
	default: return false;
	}
}

// ===================================================================
// AND
// ===================================================================

UBehaviacBehaviorTask* UBehaviacAnd::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacAndTask>(Outer);
}

EBehaviacStatus UBehaviacAndTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	for (UBehaviacBehaviorTask* Child : ChildTasks)
	{
		if (!Child) continue;

		EBehaviacStatus Result = Child->Execute(Agent, EBehaviacStatus::Invalid);
		if (Result != EBehaviacStatus::Success)
		{
			return Result; // Failure or Running propagates
		}
	}
	return EBehaviacStatus::Success;
}

// ===================================================================
// OR
// ===================================================================

UBehaviacBehaviorTask* UBehaviacOr::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacOrTask>(Outer);
}

EBehaviacStatus UBehaviacOrTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	for (UBehaviacBehaviorTask* Child : ChildTasks)
	{
		if (!Child) continue;

		EBehaviacStatus Result = Child->Execute(Agent, EBehaviacStatus::Invalid);
		if (Result == EBehaviacStatus::Success)
		{
			return EBehaviacStatus::Success;
		}
		if (Result == EBehaviacStatus::Running)
		{
			return EBehaviacStatus::Running;
		}
	}
	return EBehaviacStatus::Failure;
}

// ===================================================================
// TRUE / FALSE
// ===================================================================

UBehaviacBehaviorTask* UBehaviacTrue::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacTrueTask>(Outer);
}

EBehaviacStatus UBehaviacTrueTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	return EBehaviacStatus::Success;
}

UBehaviacBehaviorTask* UBehaviacFalse::CreateTask(UObject* Outer) const
{
	return NewObject<UBehaviacFalseTask>(Outer);
}

EBehaviacStatus UBehaviacFalseTask::OnUpdate(UBehaviacAgentComponent* Agent, EBehaviacStatus ChildStatus)
{
	return EBehaviacStatus::Failure;
}
