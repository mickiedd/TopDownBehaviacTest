// Behaviac UE5 Plugin — Decorator Node Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.Decorators

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ===========================================================================
// AlwaysSuccess
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_AlwaysSuccess_FromFailure,
	"BehaviacPlugin.Decorators.AlwaysSuccess.FromFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_AlwaysSuccess_FromFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorAlwaysSuccess* Dec =
		BT_WrapDecorator<UBehaviacDecoratorAlwaysSuccess>(BT_MakeFalse());
	TestEqual(TEXT("AlwaysSuccess wrapping Failure → Success"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_AlwaysSuccess_PassRunning,
	"BehaviacPlugin.Decorators.AlwaysSuccess.PassRunning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_AlwaysSuccess_PassRunning::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorAlwaysSuccess* Dec =
		BT_WrapDecorator<UBehaviacDecoratorAlwaysSuccess>(
			BT_MakeAction(A, EBehaviacStatus::Running, TEXT("AS_Run")));
	// Running should pass through (decorator only modifies terminal results)
	TestEqual(TEXT("AlwaysSuccess wrapping Running → Running"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// AlwaysFailure
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_AlwaysFailure_FromSuccess,
	"BehaviacPlugin.Decorators.AlwaysFailure.FromSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_AlwaysFailure_FromSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorAlwaysFailure* Dec =
		BT_WrapDecorator<UBehaviacDecoratorAlwaysFailure>(BT_MakeTrue());
	TestEqual(TEXT("AlwaysFailure wrapping Success → Failure"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_AlwaysFailure_PassRunning,
	"BehaviacPlugin.Decorators.AlwaysFailure.PassRunning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_AlwaysFailure_PassRunning::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorAlwaysFailure* Dec =
		BT_WrapDecorator<UBehaviacDecoratorAlwaysFailure>(
			BT_MakeAction(A, EBehaviacStatus::Running, TEXT("AF_Run")));
	TestEqual(TEXT("AlwaysFailure wrapping Running → Running"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// Not (Inverter)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Not_InvertSuccess,
	"BehaviacPlugin.Decorators.Not.InvertSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Not_InvertSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorNot* Dec = BT_WrapDecorator<UBehaviacDecoratorNot>(BT_MakeTrue());
	TestEqual(TEXT("Not(Success) → Failure"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Not_InvertFailure,
	"BehaviacPlugin.Decorators.Not.InvertFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Not_InvertFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorNot* Dec = BT_WrapDecorator<UBehaviacDecoratorNot>(BT_MakeFalse());
	TestEqual(TEXT("Not(Failure) → Success"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Not_PassRunning,
	"BehaviacPlugin.Decorators.Not.PassRunning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Not_PassRunning::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorNot* Dec =
		BT_WrapDecorator<UBehaviacDecoratorNot>(
			BT_MakeAction(A, EBehaviacStatus::Running, TEXT("Not_Run")));
	TestEqual(TEXT("Not(Running) → Running"),
		BT_ExecOnce(Dec, A), EBehaviacStatus::Running);
	return true;
}

// ===========================================================================
// Loop
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Loop_RunsNTimes,
	"BehaviacPlugin.Decorators.Loop.RunsNTimes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Loop_RunsNTimes::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	int32 CallCount = 0;
	A->RegisterMethodHandler(TEXT("LoopAction"), [&CallCount]() -> EBehaviacStatus
	{
		CallCount++;
		return EBehaviacStatus::Success;
	});
	UBehaviacAction* ActionNode = NewObject<UBehaviacAction>(GetTransientPackage());
	ActionNode->MethodName   = TEXT("LoopAction");
	ActionNode->ResultOption = EBehaviacStatus::Success;

	UBehaviacDecoratorLoop* Loop = BT_WrapDecorator<UBehaviacDecoratorLoop>(ActionNode);
	Loop->LoopCount = 3;

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Loop);

	// A Loop(3) around a Success child should tick Running for 3 iterations, then Success
	// Tick until not-Running (max 10 guards)
	EBehaviacStatus S = EBehaviacStatus::Running;
	int32 TickCount = 0;
	while (S == EBehaviacStatus::Running && TickCount < 20)
	{
		S = Tree->Tick(A);
		TickCount++;
	}
	TestEqual(TEXT("Loop(3) ultimately returns Success"), S, EBehaviacStatus::Success);
	TestEqual(TEXT("Loop(3) child called exactly 3 times"), CallCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Loop_Infinite,
	"BehaviacPlugin.Decorators.Loop.Infinite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Loop_Infinite::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorLoop* Loop = BT_WrapDecorator<UBehaviacDecoratorLoop>(BT_MakeNoop());
	Loop->LoopCount = -1; // infinite

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Loop);

	// Tick 10 times — should always return Running
	for (int32 i = 0; i < 10; i++)
	{
		EBehaviacStatus S = Tree->Tick(A);
		TestEqual(FString::Printf(TEXT("Loop(-1) tick %d → Running"), i + 1),
			S, EBehaviacStatus::Running);
	}
	return true;
}

// ===========================================================================
// LoopUntil
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_LoopUntil_UntilSuccess,
	"BehaviacPlugin.Decorators.LoopUntil.UntilSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_LoopUntil_UntilSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	int32 CallCount = 0;
	// Fail first 2 calls, succeed on 3rd
	A->RegisterMethodHandler(TEXT("LU_Action"), [&CallCount]() -> EBehaviacStatus
	{
		CallCount++;
		return (CallCount >= 3) ? EBehaviacStatus::Success : EBehaviacStatus::Failure;
	});
	UBehaviacAction* ActionNode = NewObject<UBehaviacAction>(GetTransientPackage());
	ActionNode->MethodName   = TEXT("LU_Action");
	ActionNode->ResultOption = EBehaviacStatus::Failure;

	UBehaviacDecoratorLoopUntil* LU = BT_WrapDecorator<UBehaviacDecoratorLoopUntil>(ActionNode);
	LU->bUntilSuccess = true;

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(LU);

	EBehaviacStatus S = EBehaviacStatus::Running;
	int32 Guard = 0;
	while (S == EBehaviacStatus::Running && Guard++ < 20)
	{
		S = Tree->Tick(A);
	}
	TestEqual(TEXT("LoopUntil returns Success when child succeeds"), S, EBehaviacStatus::Success);
	TestEqual(TEXT("Child called 3 times (failed 2x, then succeeded)"), CallCount, 3);
	return true;
}

// ===========================================================================
// Repeat
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Repeat_RunsNTimes,
	"BehaviacPlugin.Decorators.Repeat.RunsNTimes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Repeat_RunsNTimes::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	int32 RepeatCount = 0;
	A->RegisterMethodHandler(TEXT("RepeatAction"), [&RepeatCount]() -> EBehaviacStatus
	{
		RepeatCount++;
		return EBehaviacStatus::Success;
	});
	UBehaviacAction* ActionNode = NewObject<UBehaviacAction>(GetTransientPackage());
	ActionNode->MethodName   = TEXT("RepeatAction");
	ActionNode->ResultOption = EBehaviacStatus::Success;

	UBehaviacDecoratorRepeat* Rep = BT_WrapDecorator<UBehaviacDecoratorRepeat>(ActionNode);
	Rep->RepeatCount = 3;

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Rep);

	EBehaviacStatus S = EBehaviacStatus::Running;
	int32 Guard = 0;
	while (S == EBehaviacStatus::Running && Guard++ < 20)
	{
		S = Tree->Tick(A);
	}
	TestEqual(TEXT("Repeat(3) → Success"), S, EBehaviacStatus::Success);
	TestEqual(TEXT("Child ran 3 times"), RepeatCount, 3);
	return true;
}

// ===========================================================================
// CountLimit
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_CountLimit_FirstAllowed,
	"BehaviacPlugin.Decorators.CountLimit.FirstAllowed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_CountLimit_FirstAllowed::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacDecoratorCountLimit* CL = BT_WrapDecorator<UBehaviacDecoratorCountLimit>(BT_MakeTrue());
	CL->CountMax = 2;
	// First execution should succeed (under limit)
	EBehaviacStatus S = BT_ExecOnce(CL, A);
	TestTrue(TEXT("CountLimit(2) first entry allowed"),
		S == EBehaviacStatus::Success || S == EBehaviacStatus::Running);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_CountLimit_LimitReached,
	"BehaviacPlugin.Decorators.CountLimit.LimitReached",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_CountLimit_LimitReached::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	// CountMax=1 means only 1 entry allowed; the 2nd should fail
	UBehaviacDecoratorCountLimit* CL = BT_WrapDecorator<UBehaviacDecoratorCountLimit>(BT_MakeTrue());
	CL->CountMax = 1;

	// Build the tree once and tick twice (Enter is checked each time the BT re-enters)
	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(CL);
	EBehaviacStatus First  = Tree->Tick(A); // enters, runs → Success (count=1)
	EBehaviacStatus Second = Tree->Tick(A); // tries to enter, count already at limit → Failure
	TestTrue(TEXT("CountLimit first allowed"),
		First == EBehaviacStatus::Success || First == EBehaviacStatus::Running);
	TestEqual(TEXT("CountLimit second blocked → Failure"), Second, EBehaviacStatus::Failure);
	return true;
}

// ===========================================================================
// DecoratorTime — needs UWorld, skipped in automation context
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Time_Skipped,
	"BehaviacPlugin.Decorators.Time",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Time_Skipped::RunTest(const FString&)
{
	// DecoratorTime relies on UWorld::GetTimeSeconds() which is unavailable
	// in the Automation context without a running PIE world. Verified manually.
	// The implementation fix (BehaviacDecorators.cpp) corrects the copy-paste bug
	// where the result always returned Running. This test serves as a placeholder.
	AddInfo(TEXT("DecoratorTime test skipped: requires UWorld (PIE/game context)."));
	return true;
}

// ===========================================================================
// Weight (pass-through)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacDecorators_Weight_PassThrough,
	"BehaviacPlugin.Decorators.Weight.PassThrough",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacDecorators_Weight_PassThrough::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	// Weight wrapping Success → Success
	UBehaviacDecoratorWeight* WS = BT_WrapDecorator<UBehaviacDecoratorWeight>(BT_MakeTrue());
	WS->Weight = 0.5f;
	TestEqual(TEXT("Weight(Success) → Success"), BT_ExecOnce(WS, A), EBehaviacStatus::Success);

	// Weight wrapping Failure → Failure
	UBehaviacDecoratorWeight* WF = BT_WrapDecorator<UBehaviacDecoratorWeight>(BT_MakeFalse());
	WF->Weight = 0.5f;
	TestEqual(TEXT("Weight(Failure) → Failure"), BT_ExecOnce(WF, A), EBehaviacStatus::Failure);
	return true;
}
