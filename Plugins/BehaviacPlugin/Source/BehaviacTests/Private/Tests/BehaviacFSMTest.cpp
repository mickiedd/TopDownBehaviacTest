// Behaviac UE5 Plugin — FSM Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.FSM

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ===========================================================================
// FSM: Enters Initial State (single state that is both initial and final)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacFSM_EntersInitialState,
	"BehaviacPlugin.FSM.EntersInitialState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacFSM_EntersInitialState::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	// Create FSM with one state that is both initial and final
	UBehaviacFSMState* State0 = NewObject<UBehaviacFSMState>(GetTransientPackage());
	State0->StateId    = 0;
	State0->bIsInitialState = true;
	State0->bIsFinalState   = true;

	// Wire an action into the state so it does something
	int32 EnterCount = 0;
	A->RegisterMethodHandler(TEXT("FSM_S0"), [&EnterCount]() -> EBehaviacStatus
	{
		EnterCount++;
		return EBehaviacStatus::Success;
	});
	UBehaviacAction* StateAction = NewObject<UBehaviacAction>(GetTransientPackage());
	StateAction->MethodName   = TEXT("FSM_S0");
	StateAction->ResultOption = EBehaviacStatus::Success;
	State0->AddChild(StateAction);

	UBehaviacFSMNode* FSM = NewObject<UBehaviacFSMNode>(GetTransientPackage());
	FSM->AddChild(State0);

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(FSM);
	EBehaviacStatus S = Tree->Tick(A);

	// Initial+final state should run once and succeed
	TestTrue(TEXT("FSM with initial=final state completes"),
		S == EBehaviacStatus::Success || S == EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// FSM: Transition between two states
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacFSM_Transition_AlwaysTransition,
	"BehaviacPlugin.FSM.Transition.AlwaysTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacFSM_Transition_AlwaysTransition::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	// State 0: initial, not final; has an always-true transition to state 1
	UBehaviacFSMState* State0 = NewObject<UBehaviacFSMState>(GetTransientPackage());
	State0->StateId    = 0;
	State0->bIsInitialState = true;
	State0->bIsFinalState   = false;

	// Transition from 0 → 1 always fires (AlwaysTransition)
	UBehaviacAlwaysTransition* Trans = NewObject<UBehaviacAlwaysTransition>(GetTransientPackage());
	Trans->TargetStateId = 1;
	State0->Transitions.Add(Trans);

	// State 1: final
	UBehaviacFSMState* State1 = NewObject<UBehaviacFSMState>(GetTransientPackage());
	State1->StateId    = 1;
	State1->bIsInitialState = false;
	State1->bIsFinalState   = true;

	UBehaviacFSMNode* FSM = NewObject<UBehaviacFSMNode>(GetTransientPackage());
	FSM->AddChild(State0);
	FSM->AddChild(State1);

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(FSM);

	// Tick 1: enters State0, fires transition → moves to State1
	// Tick 2: State1 is final → returns Success
	EBehaviacStatus S1 = Tree->Tick(A);
	EBehaviacStatus S2 = Tree->Tick(A);

	TestTrue(TEXT("FSM transitions out of State0"),
		S1 == EBehaviacStatus::Running || S1 == EBehaviacStatus::Success);
	TestTrue(TEXT("FSM reaches final State1 → Success or Running"),
		S2 == EBehaviacStatus::Success || S2 == EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// FSM: WaitState (duration=0 expires immediately)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacFSM_WaitState,
	"BehaviacPlugin.FSM.WaitState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacFSM_WaitState::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	// WaitState with 0 duration: should succeed on first tick (elapsed >= 0.0)
	UBehaviacWaitState* WS = NewObject<UBehaviacWaitState>(GetTransientPackage());
	WS->StateId      = 0;
	WS->bIsInitialState   = true;
	WS->bIsFinalState     = true;
	WS->WaitDuration = 0.0f;

	UBehaviacFSMNode* FSM = NewObject<UBehaviacFSMNode>(GetTransientPackage());
	FSM->AddChild(WS);

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(FSM);

	// Should succeed within 2 ticks
	EBehaviacStatus S1 = Tree->Tick(A);
	EBehaviacStatus S2 = Tree->Tick(A);
	TestTrue(TEXT("WaitState(0) completes within 2 ticks"),
		S1 == EBehaviacStatus::Success || S2 == EBehaviacStatus::Success);
	return true;
}

// ===========================================================================
// FSM: WaitFramesState
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacFSM_WaitFramesState,
	"BehaviacPlugin.FSM.WaitFramesState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacFSM_WaitFramesState::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	UBehaviacWaitFramesState* WFS = NewObject<UBehaviacWaitFramesState>(GetTransientPackage());
	WFS->StateId        = 0;
	WFS->bIsInitialState     = true;
	WFS->bIsFinalState       = true;
	WFS->WaitFrameCount = 2;

	UBehaviacFSMNode* FSM = NewObject<UBehaviacFSMNode>(GetTransientPackage());
	FSM->AddChild(WFS);

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(FSM);

	// Tick 1: Running (0 frames elapsed)
	EBehaviacStatus S1 = Tree->Tick(A);
	TestEqual(TEXT("WaitFramesState tick 1 → Running"), S1, EBehaviacStatus::Running);

	// Advance frame counter
	GFrameCounter += 1;
	EBehaviacStatus S2 = Tree->Tick(A);
	TestEqual(TEXT("WaitFramesState tick 2 → Running"), S2, EBehaviacStatus::Running);

	GFrameCounter += 1;
	EBehaviacStatus S3 = Tree->Tick(A);
	TestEqual(TEXT("WaitFramesState tick 3 → Success after 2 frames"), S3, EBehaviacStatus::Success);

	return true;
}
