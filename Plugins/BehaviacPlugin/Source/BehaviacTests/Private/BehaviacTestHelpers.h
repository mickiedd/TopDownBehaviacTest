// Behaviac UE5 Plugin — Test Helpers
// Licensed under the BSD 3-Clause License.

#pragma once

#include "CoreMinimal.h"
#include "BehaviacAgent.h"
#include "BehaviacTypes.h"
#include "BehaviorTree/BehaviacBehaviorNode.h"
#include "BehaviorTree/BehaviacBehaviorTask.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "BehaviorTree/Composites/BehaviacComposites.h"
#include "BehaviorTree/Actions/BehaviacActions.h"
#include "BehaviorTree/Conditions/BehaviacConditions.h"
#include "BehaviorTree/Decorators/BehaviacDecorators.h"
#include "BehaviorTree/Attachments/BehaviacAttachment.h"
#include "FSM/BehaviacFSM.h"

// -----------------------------------------------------------------------
// Core helpers
// -----------------------------------------------------------------------

/** Create a lightweight agent component not attached to any actor. */
static UBehaviacAgentComponent* BT_MakeAgent()
{
	UBehaviacAgentComponent* Agent = NewObject<UBehaviacAgentComponent>(GetTransientPackage());
	Agent->bAutoTick = false;
	return Agent;
}

/**
 * Build a UBehaviacBehaviorTreeTask from a root node.
 * UBehaviacBehaviorTreeTask::Init() builds the full child-task hierarchy.
 */
static UBehaviacBehaviorTreeTask* BT_BuildTree(UBehaviacBehaviorNode* RootNode)
{
	UBehaviacBehaviorTreeTask* TreeTask = NewObject<UBehaviacBehaviorTreeTask>(GetTransientPackage());
	TreeTask->Init(RootNode);
	return TreeTask;
}

/**
 * Build and execute one task directly (bypasses the BehaviorTreeTask wrapper).
 * Useful for leaf/composite unit tests.
 */
static EBehaviacStatus BT_ExecOnce(UBehaviacBehaviorNode* Node, UBehaviacAgentComponent* Agent)
{
	UBehaviacBehaviorTask* Task = Node->CreateTask(GetTransientPackage());
	Task->Init(Node);
	// Pass Invalid so composites treat this as a fresh start (not mid-flight Running).
	return Task->Execute(Agent, EBehaviacStatus::Invalid);
}

/** Tick a tree task N times; returns last status. */
static EBehaviacStatus BT_TickN(UBehaviacBehaviorTreeTask* Tree, UBehaviacAgentComponent* Agent, int32 N)
{
	EBehaviacStatus Last = EBehaviacStatus::Invalid;
	for (int32 i = 0; i < N; i++)
	{
		Last = Tree->Tick(Agent);
	}
	return Last;
}

// -----------------------------------------------------------------------
// Node factory helpers
// -----------------------------------------------------------------------

/** Action leaf that registers its own method handler so the agent can return the given status. */
static UBehaviacAction* BT_MakeAction(
	UBehaviacAgentComponent* Agent,
	EBehaviacStatus ReturnStatus,
	const FString& MethodName = TEXT("TestMethod"))
{
	UBehaviacAction* Node = NewObject<UBehaviacAction>(GetTransientPackage());
	Node->MethodName = MethodName;
	// ResultOption is the fallback when no handler is registered;
	// when a handler IS registered that handler's return value takes priority
	// (unless the action's ResultOption forces a fixed value — see BehaviacActions.cpp).
	// For these tests we want the method handler to win, so set ResultOption to Invalid
	// and let the handler return what we want.
	Node->ResultOption = ReturnStatus;

	// Register a handler that returns the desired status so the BT actually exercises it
	Agent->RegisterMethodHandler(MethodName, [ReturnStatus]() -> EBehaviacStatus
	{
		return ReturnStatus;
	});
	return Node;
}

/** Noop leaf — always succeeds, no method call. */
static UBehaviacNoop* BT_MakeNoop()
{
	return NewObject<UBehaviacNoop>(GetTransientPackage());
}

/** True condition leaf — always Success. */
static UBehaviacTrue* BT_MakeTrue()
{
	return NewObject<UBehaviacTrue>(GetTransientPackage());
}

/** False condition leaf — always Failure. */
static UBehaviacFalse* BT_MakeFalse()
{
	return NewObject<UBehaviacFalse>(GetTransientPackage());
}

/** Condition node comparing two literal string operands. */
static UBehaviacCondition* BT_MakeCondition(
	const FString& Left,
	EBehaviacOperatorType Op,
	const FString& Right)
{
	UBehaviacCondition* Node = NewObject<UBehaviacCondition>(GetTransientPackage());
	Node->LeftOperand  = Left;
	Node->Operator     = Op;
	Node->RightOperand = Right;
	return Node;
}

/** Selector composite with pre-wired children. */
static UBehaviacSelector* BT_MakeSelector(TArray<UBehaviacBehaviorNode*> Kids)
{
	UBehaviacSelector* Sel = NewObject<UBehaviacSelector>(GetTransientPackage());
	for (auto* K : Kids) Sel->AddChild(K);
	return Sel;
}

/** Sequence composite with pre-wired children. */
static UBehaviacSequence* BT_MakeSequence(TArray<UBehaviacBehaviorNode*> Kids)
{
	UBehaviacSequence* Seq = NewObject<UBehaviacSequence>(GetTransientPackage());
	for (auto* K : Kids) Seq->AddChild(K);
	return Seq;
}

/** Wrap a node in a decorator, returns the decorator node with child wired. */
template<typename TDecoratorNode>
static TDecoratorNode* BT_WrapDecorator(UBehaviacBehaviorNode* Child)
{
	TDecoratorNode* Dec = NewObject<TDecoratorNode>(GetTransientPackage());
	Dec->AddChild(Child);
	return Dec;
}
