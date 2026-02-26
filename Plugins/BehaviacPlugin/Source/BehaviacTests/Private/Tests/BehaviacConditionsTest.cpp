// Behaviac UE5 Plugin — Condition Node Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.Conditions

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ---------------------------------------------------------------------------
// True / False
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_True,
	"BehaviacPlugin.Conditions.True",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_True::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("True returns Success"),
		BT_ExecOnce(BT_MakeTrue(), A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_False,
	"BehaviacPlugin.Conditions.False",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_False::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("False returns Failure"),
		BT_ExecOnce(BT_MakeFalse(), A), EBehaviacStatus::Failure);
	return true;
}

// ---------------------------------------------------------------------------
// Condition — literal comparisons
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Equal_Match,
	"BehaviacPlugin.Conditions.Condition.Equal.Match",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Equal_Match::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("5==5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("5"), EBehaviacOperatorType::Equal, TEXT("5")), A),
		EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Equal_NoMatch,
	"BehaviacPlugin.Conditions.Condition.Equal.NoMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Equal_NoMatch::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("5==6 → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("5"), EBehaviacOperatorType::Equal, TEXT("6")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_NotEqual,
	"BehaviacPlugin.Conditions.Condition.NotEqual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_NotEqual::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("5!=6 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("5"), EBehaviacOperatorType::NotEqual, TEXT("6")), A),
		EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Greater,
	"BehaviacPlugin.Conditions.Condition.Greater",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Greater::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("10>5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("10"), EBehaviacOperatorType::Greater, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("3>5 → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("3"), EBehaviacOperatorType::Greater, TEXT("5")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Less,
	"BehaviacPlugin.Conditions.Condition.Less",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Less::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("3<5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("3"), EBehaviacOperatorType::Less, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("10<5 → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("10"), EBehaviacOperatorType::Less, TEXT("5")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_GreaterEqual,
	"BehaviacPlugin.Conditions.Condition.GreaterEqual.Equal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_GreaterEqual::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("5>=5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("5"), EBehaviacOperatorType::GreaterEqual, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("6>=5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("6"), EBehaviacOperatorType::GreaterEqual, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("4>=5 → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("4"), EBehaviacOperatorType::GreaterEqual, TEXT("5")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_LessEqual,
	"BehaviacPlugin.Conditions.Condition.LessEqual.Less",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_LessEqual::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("3<=5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("3"), EBehaviacOperatorType::LessEqual, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("5<=5 → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("5"), EBehaviacOperatorType::LessEqual, TEXT("5")), A),
		EBehaviacStatus::Success);
	TestEqual(TEXT("6<=5 → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("6"), EBehaviacOperatorType::LessEqual, TEXT("5")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_StringEqual,
	"BehaviacPlugin.Conditions.Condition.StringEqual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_StringEqual::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("hello==hello → Success"),
		BT_ExecOnce(BT_MakeCondition(TEXT("hello"), EBehaviacOperatorType::Equal, TEXT("hello")), A),
		EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_StringNotEqual,
	"BehaviacPlugin.Conditions.Condition.StringNotEqual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_StringNotEqual::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestEqual(TEXT("hello==world → Failure"),
		BT_ExecOnce(BT_MakeCondition(TEXT("hello"), EBehaviacOperatorType::Equal, TEXT("world")), A),
		EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_PropertyRef,
	"BehaviacPlugin.Conditions.Condition.PropertyRef",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_PropertyRef::RunTest(const FString&)
{
	// Test that "Self.HP" resolves to the agent property "HP"
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("HP"), TEXT("10"));
	// Condition: Self.HP > 5
	UBehaviacCondition* Node = BT_MakeCondition(TEXT("Self.HP"), EBehaviacOperatorType::Greater, TEXT("5"));
	EBehaviacStatus S = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Self.HP(10) > 5 → Success"), S, EBehaviacStatus::Success);

	A->SetPropertyValue(TEXT("HP"), TEXT("3"));
	// Reset the node's task by creating a fresh one
	EBehaviacStatus S2 = BT_ExecOnce(Node, A);
	TestEqual(TEXT("Self.HP(3) > 5 → Failure"), S2, EBehaviacStatus::Failure);
	return true;
}

// ---------------------------------------------------------------------------
// And / Or
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_And_AllSuccess,
	"BehaviacPlugin.Conditions.And.AllSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_And_AllSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacAnd* And = NewObject<UBehaviacAnd>(GetTransientPackage());
	And->AddChild(BT_MakeTrue());
	And->AddChild(BT_MakeTrue());
	TestEqual(TEXT("And(True,True) → Success"),
		BT_ExecOnce(And, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_And_OneFail,
	"BehaviacPlugin.Conditions.And.OneFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_And_OneFail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacAnd* And = NewObject<UBehaviacAnd>(GetTransientPackage());
	And->AddChild(BT_MakeTrue());
	And->AddChild(BT_MakeFalse());
	TestEqual(TEXT("And(True,False) → Failure"),
		BT_ExecOnce(And, A), EBehaviacStatus::Failure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Or_OneSuccess,
	"BehaviacPlugin.Conditions.Or.OneSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Or_OneSuccess::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacOr* Or = NewObject<UBehaviacOr>(GetTransientPackage());
	Or->AddChild(BT_MakeFalse());
	Or->AddChild(BT_MakeTrue());
	TestEqual(TEXT("Or(False,True) → Success"),
		BT_ExecOnce(Or, A), EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacConditions_Or_AllFail,
	"BehaviacPlugin.Conditions.Or.AllFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacConditions_Or_AllFail::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	UBehaviacOr* Or = NewObject<UBehaviacOr>(GetTransientPackage());
	Or->AddChild(BT_MakeFalse());
	Or->AddChild(BT_MakeFalse());
	TestEqual(TEXT("Or(False,False) → Failure"),
		BT_ExecOnce(Or, A), EBehaviacStatus::Failure);
	return true;
}
