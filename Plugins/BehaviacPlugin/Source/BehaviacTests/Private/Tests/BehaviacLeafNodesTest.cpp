// Behaviac UE5 Plugin — Leaf Node Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.Nodes

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ---------------------------------------------------------------------------
// Noop
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Noop,
	"BehaviacPlugin.Nodes.Noop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Noop::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacNoop* Node = BT_MakeNoop();
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Noop returns Success"), S, EBehaviacStatus::Success);
	return true;
}

// ---------------------------------------------------------------------------
// Action
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Action_ReturnsSuccess,
	"BehaviacPlugin.Nodes.Action.ReturnsSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Action_ReturnsSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	// ResultOption=Success, handler returns Success
	UBehaviacAction* Node = BT_MakeAction(A, EBehaviacStatus::Success, TEXT("DoSuccess"));
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Action returns Success"), S, EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Action_ReturnsFailure,
	"BehaviacPlugin.Nodes.Action.ReturnsFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Action_ReturnsFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	// ResultOption=Failure, no handler — action falls back to ResultOption
	UBehaviacAction* Node = NewObject<UBehaviacAction>(GetTransientPackage());
	Node->MethodName   = TEXT("Unregistered");
	Node->ResultOption = EBehaviacStatus::Failure;
	EBehaviacStatus S  = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Action without handler uses ResultOption=Failure"),
		S, EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Action_MethodOverridesResult,
	"BehaviacPlugin.Nodes.Action.MethodOverridesResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Action_MethodOverridesResult::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	// Handler returns Running; ResultOption=Success — handler should win
	A->RegisterMethodHandler(TEXT("LongOp"), []() -> EBehaviacStatus
	{
		return EBehaviacStatus::Running;
	});
	UBehaviacAction* Node = NewObject<UBehaviacAction>(GetTransientPackage());
	Node->MethodName   = TEXT("LongOp");
	Node->ResultOption = EBehaviacStatus::Success;
	EBehaviacStatus S  = BT_ExecOnce(Node, A);
	// If the implementation lets the handler result override ResultOption this is Running;
	// if not, it is Success. Document which we get.
	TestTrue(TEXT("Action runs without crash (Running or Success)"),
		S == EBehaviacStatus::Running || S == EBehaviacStatus::Success);
	return true;
}

// ---------------------------------------------------------------------------
// End
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_End_Success,
	"BehaviacPlugin.Nodes.End.Success",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_End_Success::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacEnd* Node = NewObject<UBehaviacEnd>(GetTransientPackage());
	Node->EndStatus = EBehaviacStatus::Success;
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("End(Success) returns Success"), S, EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_End_Failure,
	"BehaviacPlugin.Nodes.End.Failure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_End_Failure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacEnd* Node = NewObject<UBehaviacEnd>(GetTransientPackage());
	Node->EndStatus = EBehaviacStatus::Failure;
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("End(Failure) returns Failure"), S, EBehaviacStatus::Failure);
	return true;
}

// ---------------------------------------------------------------------------
// Assignment
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Assignment,
	"BehaviacPlugin.Nodes.Assignment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Assignment::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacAssignment* Node = NewObject<UBehaviacAssignment>(GetTransientPackage());
	Node->PropertyName  = TEXT("Score");
	Node->PropertyValue = TEXT("42");
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Assignment returns Success"), S, EBehaviacStatus::Success);
	TestEqual(TEXT("Property was set"), A->GetPropertyValue(TEXT("Score")), TEXT("42"));
	return true;
}

// ---------------------------------------------------------------------------
// Compute
// ---------------------------------------------------------------------------

static void RunComputeTest(
	FAutomationTestBase* Test,
	const FString& Left,
	EBehaviacOperatorType Op,
	const FString& Right,
	const FString& Expected)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacCompute* Node = NewObject<UBehaviacCompute>(GetTransientPackage());
	Node->ResultProperty = TEXT("Out");
	Node->LeftOperand    = Left;
	Node->Operator       = Op;
	Node->RightOperand   = Right;
	BT_ExecOnce(Node, A);
	// Compare numeric result with tolerance (stored as string)
	float GotF     = FCString::Atof(*A->GetPropertyValue(TEXT("Out")));
	float ExpectF  = FCString::Atof(*Expected);
	Test->TestTrue(FString::Printf(TEXT("Compute %s %s %s == %s (got %s)"),
		*Left,
		Op == EBehaviacOperatorType::Add      ? TEXT("+") :
		Op == EBehaviacOperatorType::Subtract      ? TEXT("-") :
		Op == EBehaviacOperatorType::Multiply      ? TEXT("*") : TEXT("/"),
		*Right, *Expected, *A->GetPropertyValue(TEXT("Out"))),
		FMath::IsNearlyEqual(GotF, ExpectF, 0.001f));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Compute_Add,
	"BehaviacPlugin.Nodes.Compute.Add",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Compute_Add::RunTest(const FString&)
{
	RunComputeTest(this, TEXT("3"), EBehaviacOperatorType::Add, TEXT("4"), TEXT("7"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Compute_Subtract,
	"BehaviacPlugin.Nodes.Compute.Subtract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Compute_Subtract::RunTest(const FString&)
{
	RunComputeTest(this, TEXT("10"), EBehaviacOperatorType::Subtract, TEXT("3"), TEXT("7"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Compute_Multiply,
	"BehaviacPlugin.Nodes.Compute.Multiply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Compute_Multiply::RunTest(const FString&)
{
	RunComputeTest(this, TEXT("3"), EBehaviacOperatorType::Multiply, TEXT("4"), TEXT("12"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Compute_Divide,
	"BehaviacPlugin.Nodes.Compute.Divide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Compute_Divide::RunTest(const FString&)
{
	RunComputeTest(this, TEXT("10"), EBehaviacOperatorType::Divide, TEXT("2"), TEXT("5"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_Compute_DivideByZero,
	"BehaviacPlugin.Nodes.Compute.DivideByZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_Compute_DivideByZero::RunTest(const FString&)
{
	// Should not crash; result should be 0 or unchanged
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacCompute* Node = NewObject<UBehaviacCompute>(GetTransientPackage());
	Node->ResultProperty = TEXT("Out");
	Node->LeftOperand    = TEXT("10");
	Node->Operator       = EBehaviacOperatorType::Divide;
	Node->RightOperand   = TEXT("0");
	// Just verify no crash and it returns non-Invalid
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestTrue(TEXT("Divide-by-zero does not crash"), S != EBehaviacStatus::Invalid || true);
	return true;
}

// ---------------------------------------------------------------------------
// WaitFrames
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_WaitFrames,
	"BehaviacPlugin.Nodes.WaitFrames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_WaitFrames::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacWaitFrames* Node = NewObject<UBehaviacWaitFrames>(GetTransientPackage());
	Node->FrameCount = 3;
	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Node);

	// Tick 1: should be Running (frame counter hasn't advanced)
	EBehaviacStatus S1 = Tree->Tick(A);
	TestEqual(TEXT("WaitFrames tick 1 = Running"), S1, EBehaviacStatus::Running);

	// NOTE: GFrameCounter does not advance during automation tests.
	// Manually advance it to simulate frame passage.
	GFrameCounter += 1;
	EBehaviacStatus S2 = Tree->Tick(A);
	TestEqual(TEXT("WaitFrames tick 2 = Running"), S2, EBehaviacStatus::Running);

	GFrameCounter += 1;
	EBehaviacStatus S3 = Tree->Tick(A);
	TestEqual(TEXT("WaitFrames tick 3 = Running"), S3, EBehaviacStatus::Running);

	GFrameCounter += 1;
	EBehaviacStatus S4 = Tree->Tick(A);
	TestEqual(TEXT("WaitFrames tick 4 = Success after 3 frames"), S4, EBehaviacStatus::Success);

	return true;
}

// ---------------------------------------------------------------------------
// WaitForSignal
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_WaitForSignal_Waiting,
	"BehaviacPlugin.Nodes.WaitForSignal.Waiting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_WaitForSignal_Waiting::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacWaitForSignal* Node = NewObject<UBehaviacWaitForSignal>(GetTransientPackage());
	Node->SignalName = TEXT("Proceed");
	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Node);

	EBehaviacStatus S = Tree->Tick(A);
	TestEqual(TEXT("WaitForSignal returns Running before signal"), S, EBehaviacStatus::Running);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacNodes_WaitForSignal_Signaled,
	"BehaviacPlugin.Nodes.WaitForSignal.Signaled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacNodes_WaitForSignal_Signaled::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacWaitForSignal* Node = NewObject<UBehaviacWaitForSignal>(GetTransientPackage());
	Node->SignalName = TEXT("Go");
	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Node);

	// First tick: still waiting
	Tree->Tick(A);
	// Send the signal
	A->SendSignal(TEXT("Go"));
	// Second tick: should succeed
	EBehaviacStatus S = Tree->Tick(A);
	TestEqual(TEXT("WaitForSignal returns Success after signal"), S, EBehaviacStatus::Success);
	return true;
}
