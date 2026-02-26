// Behaviac UE5 Plugin — Attachment Tests (Preconditions & Effectors)
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.Attachments

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ===========================================================================
// Precondition — Evaluate()
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Precondition_Equal_Pass,
	"BehaviacPlugin.Attachments.Precondition.Equal.Pass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Precondition_Equal_Pass::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("X"), TEXT("5"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand  = TEXT("Self.X");
	Pre->Operator     = EBehaviacOperatorType::Equal;
	Pre->RightOperand = TEXT("5");
	Pre->bNegate      = false;

	TestTrue(TEXT("Precondition Self.X==5 passes when X=5"), Pre->Evaluate(A));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Precondition_Equal_Fail,
	"BehaviacPlugin.Attachments.Precondition.Equal.Fail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Precondition_Equal_Fail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("X"), TEXT("3"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand  = TEXT("Self.X");
	Pre->Operator     = EBehaviacOperatorType::Equal;
	Pre->RightOperand = TEXT("5");

	TestFalse(TEXT("Precondition Self.X==5 fails when X=3"), Pre->Evaluate(A));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Precondition_Greater_Pass,
	"BehaviacPlugin.Attachments.Precondition.Greater.Pass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Precondition_Greater_Pass::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("X"), TEXT("10"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand  = TEXT("Self.X");
	Pre->Operator     = EBehaviacOperatorType::Greater;
	Pre->RightOperand = TEXT("5");

	TestTrue(TEXT("Precondition Self.X>5 passes when X=10"), Pre->Evaluate(A));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Precondition_Negate,
	"BehaviacPlugin.Attachments.Precondition.Negate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Precondition_Negate::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("X"), TEXT("5"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand  = TEXT("Self.X");
	Pre->Operator     = EBehaviacOperatorType::Equal;
	Pre->RightOperand = TEXT("5");
	// Negate: 5==5 is true, negated → false
	Pre->bNegate      = true;

	TestFalse(TEXT("Negated Precondition Self.X==5 with bNegate=true → false"), Pre->Evaluate(A));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Precondition_LoadFromProperties,
	"BehaviacPlugin.Attachments.Precondition.LoadFromProperties",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Precondition_LoadFromProperties::RunTest(const FString&)
{
	// Test that LoadFromProperties correctly parses Opl/Opr/Operator
	TArray<FBehaviacProperty> Props;
	FBehaviacProperty P1, P2, P3;
	P1.Name = TEXT("Opl");      P1.Value = TEXT("Self.HP");
	P2.Name = TEXT("Operator"); P2.Value = TEXT("Greater");
	P3.Name = TEXT("Opr");      P3.Value = TEXT("0");
	Props.Add(P1); Props.Add(P2); Props.Add(P3);

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LoadFromProperties(5, TEXT("TestAgent"), Props);

	TestEqual(TEXT("LeftOperand parsed"),   Pre->LeftOperand,  TEXT("Self.HP"));
	TestEqual(TEXT("RightOperand parsed"),  Pre->RightOperand, TEXT("0"));
	TestEqual(TEXT("Operator parsed"),      Pre->Operator,     EBehaviacOperatorType::Greater);
	return true;
}

// ===========================================================================
// Effector — Apply()
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Effector_Apply,
	"BehaviacPlugin.Attachments.Effector.Apply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Effector_Apply::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	UBehaviacEffector* Eff = NewObject<UBehaviacEffector>(GetTransientPackage());
	Eff->PropertyName  = TEXT("Result");
	Eff->PropertyValue = TEXT("done");
	// EffectorPhase defaults to Both / Success — apply with bSuccess=true
	Eff->Apply(A, /*bSuccess=*/true);

	TestEqual(TEXT("Effector sets property on success"),
		A->GetPropertyValue(TEXT("Result")), TEXT("done"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_Effector_SkipOnFailure,
	"BehaviacPlugin.Attachments.Effector.SkipOnFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_Effector_SkipOnFailure::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();

	UBehaviacEffector* Eff = NewObject<UBehaviacEffector>(GetTransientPackage());
	Eff->PropertyName  = TEXT("ShouldNotBeSet");
	Eff->PropertyValue = TEXT("oops");
	// Set phase to Success-only
	Eff->EffectorPhase = EBehaviacEffectorPhase::Success;
	// Apply with failure
	Eff->Apply(A, /*bSuccess=*/false);

	TestFalse(TEXT("Effector skips property set when bSuccess=false and phase=Success"),
		A->HasProperty(TEXT("ShouldNotBeSet")));
	return true;
}

// ===========================================================================
// Precondition integrated with a node (via Preconditions array)
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_NodePrecondition_Blocks,
	"BehaviacPlugin.Attachments.NodePrecondition.BlocksExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_NodePrecondition_Blocks::RunTest(const FString&)
{
	// Attach a failing precondition to a Noop node;
	// the node should not execute (returns Failure)
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("Enabled"), TEXT("0"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand        = TEXT("Self.Enabled");
	Pre->Operator           = EBehaviacOperatorType::Equal;
	Pre->RightOperand       = TEXT("1");
	Pre->PreconditionPhase  = EBehaviacPreconditionPhase::Enter;

	UBehaviacNoop* Noop = BT_MakeNoop();
	Noop->Preconditions.Add(Pre);

	EBehaviacStatus S = BT_ExecOnce(Noop, A);
	TestEqual(TEXT("Precondition blocks Noop when condition fails → Failure"),
		S, EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAttachments_NodePrecondition_Allows,
	"BehaviacPlugin.Attachments.NodePrecondition.AllowsExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAttachments_NodePrecondition_Allows::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("Enabled"), TEXT("1"));

	UBehaviacPrecondition* Pre = NewObject<UBehaviacPrecondition>(GetTransientPackage());
	Pre->LeftOperand        = TEXT("Self.Enabled");
	Pre->Operator           = EBehaviacOperatorType::Equal;
	Pre->RightOperand       = TEXT("1");
	Pre->PreconditionPhase  = EBehaviacPreconditionPhase::Enter;

	UBehaviacNoop* Noop = BT_MakeNoop();
	Noop->Preconditions.Add(Pre);

	EBehaviacStatus S = BT_ExecOnce(Noop, A);
	TestEqual(TEXT("Precondition allows Noop when condition passes → Success"),
		S, EBehaviacStatus::Success);
	return true;
}
