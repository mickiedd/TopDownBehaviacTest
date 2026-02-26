// Behaviac UE5 Plugin — Composite Node Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.Composites

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ===========================================================================
// Helpers local to this file
// ===========================================================================

/** Make an Action node that increments a counter and returns the given status. */
static UBehaviacAction* MakeCountedAction(
	UBehaviacAgentComponent* Agent,
	int32& OutCounter,
	EBehaviacStatus ReturnStatus,
	const FString& MethodName)
{
	Agent->RegisterMethodHandler(MethodName, [&OutCounter, ReturnStatus]() -> EBehaviacStatus
	{
		OutCounter++;
		return ReturnStatus;
	});
	UBehaviacAction* Node = NewObject<UBehaviacAction>(GetTransientPackage());
	Node->MethodName   = MethodName;
	Node->ResultOption = ReturnStatus;
	return Node;
}

// ===========================================================================
// SELECTOR
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Selector_FirstSucceeds,
	"BehaviacPlugin.Composites.Selector.FirstSucceeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Selector_FirstSucceeds::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	int32 C1 = 0, C2 = 0;
	UBehaviacSelector* Sel = NewObject<UBehaviacSelector>(GetTransientPackage());
	Sel->AddChild(MakeCountedAction(A, C1, EBehaviacStatus::Success, TEXT("S1")));
	Sel->AddChild(MakeCountedAction(A, C2, EBehaviacStatus::Failure, TEXT("S2")));

	EBehaviacStatus S = BT_ExecOnce(Sel, A);
	TestEqual(TEXT("Selector returns Success"), S, EBehaviacStatus::Success);
	TestEqual(TEXT("First child ran once"), C1, 1);
	TestEqual(TEXT("Second child never ran"), C2, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Selector_FirstFailsSecondSucceeds,
	"BehaviacPlugin.Composites.Selector.FirstFailsSecondSucceeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Selector_FirstFailsSecondSucceeds::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSelector* Sel = NewObject<UBehaviacSelector>(GetTransientPackage());
	Sel->AddChild(BT_MakeFalse());
	Sel->AddChild(BT_MakeTrue());
	TestEqual(TEXT("[Fail,Success] → Success"), BT_ExecOnce(Sel, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Selector_AllFail,
	"BehaviacPlugin.Composites.Selector.AllFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Selector_AllFail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSelector* Sel = NewObject<UBehaviacSelector>(GetTransientPackage());
	Sel->AddChild(BT_MakeFalse());
	Sel->AddChild(BT_MakeFalse());
	TestEqual(TEXT("[Fail,Fail] → Failure"), BT_ExecOnce(Sel, A), EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Selector_RunningChild,
	"BehaviacPlugin.Composites.Selector.RunningChild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Selector_RunningChild::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSelector* Sel = NewObject<UBehaviacSelector>(GetTransientPackage());
	Sel->AddChild(BT_MakeAction(A, EBehaviacStatus::Running, TEXT("RunChild")));
	EBehaviacStatus S = BT_ExecOnce(Sel, A);
	TestEqual(TEXT("[Running] → Running"), S, EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// SEQUENCE
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Sequence_AllSucceed,
	"BehaviacPlugin.Composites.Sequence.AllSucceed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Sequence_AllSucceed::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSequence* Seq = NewObject<UBehaviacSequence>(GetTransientPackage());
	Seq->AddChild(BT_MakeTrue());
	Seq->AddChild(BT_MakeTrue());
	TestEqual(TEXT("[Success,Success] → Success"), BT_ExecOnce(Seq, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Sequence_FirstFails,
	"BehaviacPlugin.Composites.Sequence.FirstFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Sequence_FirstFails::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	int32 C2 = 0;
	UBehaviacSequence* Seq = NewObject<UBehaviacSequence>(GetTransientPackage());
	Seq->AddChild(BT_MakeFalse());
	Seq->AddChild(MakeCountedAction(A, C2, EBehaviacStatus::Success, TEXT("SeqC2")));
	EBehaviacStatus S = BT_ExecOnce(Seq, A);
	TestEqual(TEXT("[Fail,?] → Failure"), S, EBehaviacStatus::Failure);
	TestEqual(TEXT("Second child never ran"), C2, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Sequence_Running,
	"BehaviacPlugin.Composites.Sequence.Running",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Sequence_Running::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSequence* Seq = NewObject<UBehaviacSequence>(GetTransientPackage());
	Seq->AddChild(BT_MakeAction(A, EBehaviacStatus::Running, TEXT("RunSeq")));
	Seq->AddChild(BT_MakeTrue());
	TestEqual(TEXT("[Running,?] → Running"), BT_ExecOnce(Seq, A), EBehaviacStatus::Running);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Sequence_StopsOnFailure,
	"BehaviacPlugin.Composites.Sequence.StopsOnFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Sequence_StopsOnFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSequence* Seq = NewObject<UBehaviacSequence>(GetTransientPackage());
	Seq->AddChild(BT_MakeTrue());
	Seq->AddChild(BT_MakeFalse());
	TestEqual(TEXT("[Success,Fail] → Failure"), BT_ExecOnce(Seq, A), EBehaviacStatus::Failure);
	return true;
}

// ===========================================================================
// PARALLEL
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Parallel_AllSucceed,
	"BehaviacPlugin.Composites.Parallel.AllSucceed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Parallel_AllSucceed::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy    = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Once;
	Par->AddChild(BT_MakeTrue());
	Par->AddChild(BT_MakeTrue());
	EBehaviacStatus S = BT_ExecOnce(Par, A);
	TestEqual(TEXT("Parallel [Success,Success] → Success"), S, EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Parallel_OneFails,
	"BehaviacPlugin.Composites.Parallel.OneFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Parallel_OneFails::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy    = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Once;
	Par->AddChild(BT_MakeTrue());
	Par->AddChild(BT_MakeFalse());
	EBehaviacStatus S = BT_ExecOnce(Par, A);
	TestEqual(TEXT("Parallel FailOnOne [S,F] → Failure"), S, EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_Parallel_Running,
	"BehaviacPlugin.Composites.Parallel.Running",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_Parallel_Running::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy    = EBehaviacParallelPolicy::FailOnAll_SucceedOnOne;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Loop;
	Par->AddChild(BT_MakeAction(A, EBehaviacStatus::Running, TEXT("ParRunA")));
	Par->AddChild(BT_MakeAction(A, EBehaviacStatus::Running, TEXT("ParRunB")));
	EBehaviacStatus S = BT_ExecOnce(Par, A);
	TestEqual(TEXT("Parallel [Running,Running] → Running"), S, EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// IF-ELSE
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_IfElse_TrueBranch,
	"BehaviacPlugin.Composites.IfElse.TrueBranch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_IfElse_TrueBranch::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacIfElse* IfElse = NewObject<UBehaviacIfElse>(GetTransientPackage());
	IfElse->AddChild(BT_MakeTrue());   // condition
	IfElse->AddChild(BT_MakeTrue());   // true branch → success
	IfElse->AddChild(BT_MakeFalse());  // false branch → failure
	TestEqual(TEXT("IfElse(true) → Success"), BT_ExecOnce(IfElse, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_IfElse_FalseBranch,
	"BehaviacPlugin.Composites.IfElse.FalseBranch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_IfElse_FalseBranch::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacIfElse* IfElse = NewObject<UBehaviacIfElse>(GetTransientPackage());
	IfElse->AddChild(BT_MakeFalse());  // condition
	IfElse->AddChild(BT_MakeTrue());   // true branch
	IfElse->AddChild(BT_MakeFalse());  // false branch → failure
	TestEqual(TEXT("IfElse(false) → Failure"), BT_ExecOnce(IfElse, A), EBehaviacStatus::Failure);
	return true;
}

// ===========================================================================
// WITH PRECONDITION
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_WithPrecondition_Pass,
	"BehaviacPlugin.Composites.WithPrecondition.Pass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_WithPrecondition_Pass::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacWithPrecondition* WP = NewObject<UBehaviacWithPrecondition>(GetTransientPackage());
	WP->AddChild(BT_MakeTrue());   // precondition
	WP->AddChild(BT_MakeTrue());   // action
	TestEqual(TEXT("WithPrecondition(true) → Success"), BT_ExecOnce(WP, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_WithPrecondition_Fail,
	"BehaviacPlugin.Composites.WithPrecondition.Fail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_WithPrecondition_Fail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacWithPrecondition* WP = NewObject<UBehaviacWithPrecondition>(GetTransientPackage());
	WP->AddChild(BT_MakeFalse());  // precondition fails
	WP->AddChild(BT_MakeTrue());   // action should not run
	TestEqual(TEXT("WithPrecondition(false) → Failure"), BT_ExecOnce(WP, A), EBehaviacStatus::Failure);
	return true;
}

// ===========================================================================
// SELECTOR STOCHASTIC
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SelectorStochastic_AllFail,
	"BehaviacPlugin.Composites.SelectorStochastic.AllFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SelectorStochastic_AllFail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSelectorStochastic* SS = NewObject<UBehaviacSelectorStochastic>(GetTransientPackage());
	SS->AddChild(BT_MakeFalse());
	SS->AddChild(BT_MakeFalse());
	SS->AddChild(BT_MakeFalse());
	TestEqual(TEXT("SelectorStochastic all-fail → Failure"),
		BT_ExecOnce(SS, A), EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SelectorStochastic_HasSuccess,
	"BehaviacPlugin.Composites.SelectorStochastic.HasSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SelectorStochastic_HasSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSelectorStochastic* SS = NewObject<UBehaviacSelectorStochastic>(GetTransientPackage());
	SS->AddChild(BT_MakeFalse());
	SS->AddChild(BT_MakeTrue());
	SS->AddChild(BT_MakeFalse());
	// Whatever order they're shuffled in, one child succeeds → result is Success
	TestEqual(TEXT("SelectorStochastic [F,S,F] → Success"),
		BT_ExecOnce(SS, A), EBehaviacStatus::Success);
	return true;
}

// ===========================================================================
// SEQUENCE STOCHASTIC
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SequenceStochastic_AllSucceed,
	"BehaviacPlugin.Composites.SequenceStochastic.AllSucceed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SequenceStochastic_AllSucceed::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSequenceStochastic* SeqS = NewObject<UBehaviacSequenceStochastic>(GetTransientPackage());
	SeqS->AddChild(BT_MakeTrue());
	SeqS->AddChild(BT_MakeTrue());
	SeqS->AddChild(BT_MakeTrue());
	TestEqual(TEXT("SequenceStochastic all-success → Success"),
		BT_ExecOnce(SeqS, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SequenceStochastic_HasFailure,
	"BehaviacPlugin.Composites.SequenceStochastic.HasFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SequenceStochastic_HasFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacSequenceStochastic* SeqS = NewObject<UBehaviacSequenceStochastic>(GetTransientPackage());
	SeqS->AddChild(BT_MakeTrue());
	SeqS->AddChild(BT_MakeFalse());
	SeqS->AddChild(BT_MakeTrue());
	// Whatever order the sequence processes children, hitting the false one stops it
	TestEqual(TEXT("SequenceStochastic [S,F,S] → Failure"),
		BT_ExecOnce(SeqS, A), EBehaviacStatus::Failure);
	return true;
}

// ===========================================================================
// SELECTOR PROBABILITY (weighted)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SelectorProbability_SingleChild,
	"BehaviacPlugin.Composites.SelectorProbability.SingleChild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SelectorProbability_SingleChild::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	// Structure: SelectorProbability → DecoratorWeight(1.0) → Noop
	UBehaviacDecoratorWeight* WeightedNoop = NewObject<UBehaviacDecoratorWeight>(GetTransientPackage());
	WeightedNoop->Weight = 1.0f;
	WeightedNoop->AddChild(BT_MakeNoop());

	UBehaviacSelectorProbability* SP = NewObject<UBehaviacSelectorProbability>(GetTransientPackage());
	SP->AddChild(WeightedNoop);

	TestEqual(TEXT("SingleChild always runs → Success"), BT_ExecOnce(SP, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacComposites_SelectorProbability_WeightedSelection,
	"BehaviacPlugin.Composites.SelectorProbability.TwoChildren",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacComposites_SelectorProbability_WeightedSelection::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	int32 CountA = 0, CountB = 0;

	// Child A: weight=0 (should never be picked)
	UBehaviacDecoratorWeight* WrapA = NewObject<UBehaviacDecoratorWeight>(GetTransientPackage());
	WrapA->Weight = 0.0f;
	WrapA->AddChild(MakeCountedAction(A, CountA, EBehaviacStatus::Success, TEXT("ProbA")));

	// Child B: weight=1 (should always be picked)
	UBehaviacDecoratorWeight* WrapB = NewObject<UBehaviacDecoratorWeight>(GetTransientPackage());
	WrapB->Weight = 1.0f;
	WrapB->AddChild(MakeCountedAction(A, CountB, EBehaviacStatus::Success, TEXT("ProbB")));

	for (int32 i = 0; i < 10; i++)
	{
		UBehaviacSelectorProbability* SP = NewObject<UBehaviacSelectorProbability>(GetTransientPackage());
		SP->AddChild(WrapA);
		SP->AddChild(WrapB);
		BT_ExecOnce(SP, A);
	}

	TestEqual(TEXT("Weight=0 child never selected"), CountA, 0);
	TestEqual(TEXT("Weight=1 child always selected (10/10)"), CountB, 10);
	return true;
}
