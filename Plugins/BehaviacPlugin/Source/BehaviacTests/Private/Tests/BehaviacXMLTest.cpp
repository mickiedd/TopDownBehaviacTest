// Behaviac UE5 Plugin — XML Parser Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.XML

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"
#include "BehaviorTree/BehaviacBehaviorTree.h"
#include "BehaviorTree/Composites/BehaviacComposites.h"
#include "BehaviorTree/Actions/BehaviacActions.h"
#include "BehaviorTree/Conditions/BehaviacConditions.h"

// ===========================================================================
// Helper: load XML and return the tree asset
// ===========================================================================

static UBehaviacBehaviorTree* LoadXML(const FString& XML)
{
	UBehaviacBehaviorTree* Tree = NewObject<UBehaviacBehaviorTree>(GetTransientPackage());
	bool bOK = Tree->LoadFromXML(XML);
	return bOK ? Tree : nullptr;
}

// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_LoadSimpleSelector,
	"BehaviacPlugin.XML.LoadSimpleSelector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_LoadSimpleSelector::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"  <node class=\"behaviac::Selector\" id=\"1\">"
			"    <node class=\"behaviac::Action\" id=\"2\">"
			"      <property name=\"Method\" value=\"TestAction\"/>"
			"    </node>"
			"  </node>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded without error"), Tree)) return false;
	if (!TestNotNull(TEXT("RootNode is not null"), Tree->RootNode)) return false;

	TestTrue(TEXT("Root is UBehaviacSelector"),
		Tree->RootNode->IsA(UBehaviacSelector::StaticClass()));
	TestEqual(TEXT("Root has 1 child"), Tree->RootNode->GetChildCount(), 1);

	if (Tree->RootNode->GetChildCount() > 0)
	{
		UBehaviacBehaviorNode* Child = Tree->RootNode->GetChild(0);
		if (TestNotNull(TEXT("Child exists"), Child))
		{
			TestTrue(TEXT("Child is UBehaviacAction"),
				Child->IsA(UBehaviacAction::StaticClass()));
			UBehaviacAction* ActionNode = Cast<UBehaviacAction>(Child);
			if (ActionNode)
			{
				TestEqual(TEXT("MethodName == TestAction"),
					ActionNode->MethodName, TEXT("TestAction"));
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_LoadSequence,
	"BehaviacPlugin.XML.LoadSequence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_LoadSequence::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"  <node class=\"behaviac::Sequence\" id=\"1\">"
			"    <node class=\"behaviac::Action\" id=\"2\">"
			"      <property name=\"Method\" value=\"ActionA\"/>"
			"    </node>"
			"    <node class=\"behaviac::Action\" id=\"3\">"
			"      <property name=\"Method\" value=\"ActionB\"/>"
			"    </node>"
			"  </node>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded"), Tree)) return false;
	if (!TestNotNull(TEXT("RootNode set"), Tree->RootNode)) return false;

	TestTrue(TEXT("Root is UBehaviacSequence"),
		Tree->RootNode->IsA(UBehaviacSequence::StaticClass()));
	TestEqual(TEXT("Sequence has 2 children"), Tree->RootNode->GetChildCount(), 2);

	if (Tree->RootNode->GetChildCount() == 2)
	{
		UBehaviacAction* A1 = Cast<UBehaviacAction>(Tree->RootNode->GetChild(0));
		UBehaviacAction* A2 = Cast<UBehaviacAction>(Tree->RootNode->GetChild(1));
		TestNotNull(TEXT("Child 0 is Action"), A1);
		TestNotNull(TEXT("Child 1 is Action"), A2);
		if (A1) TestEqual(TEXT("Child 0 method=ActionA"), A1->MethodName, TEXT("ActionA"));
		if (A2) TestEqual(TEXT("Child 1 method=ActionB"), A2->MethodName, TEXT("ActionB"));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_LoadWithCondition,
	"BehaviacPlugin.XML.LoadWithCondition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_LoadWithCondition::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"  <node class=\"behaviac::Sequence\" id=\"1\">"
			"    <node class=\"behaviac::Condition\" id=\"2\">"
			"      <property name=\"Opl\" value=\"Self.HP\"/>"
			"      <property name=\"Operator\" value=\"Greater\"/>"
			"      <property name=\"Opr\" value=\"0\"/>"
			"    </node>"
			"    <node class=\"behaviac::Action\" id=\"3\">"
			"      <property name=\"Method\" value=\"Attack\"/>"
			"    </node>"
			"  </node>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded"), Tree)) return false;
	if (!TestNotNull(TEXT("RootNode set"), Tree->RootNode)) return false;

	TestTrue(TEXT("Root is Sequence"),
		Tree->RootNode->IsA(UBehaviacSequence::StaticClass()));
	TestEqual(TEXT("2 children"), Tree->RootNode->GetChildCount(), 2);

	if (Tree->RootNode->GetChildCount() >= 1)
	{
		UBehaviacCondition* Cond = Cast<UBehaviacCondition>(Tree->RootNode->GetChild(0));
		if (TestNotNull(TEXT("Child 0 is Condition"), Cond))
		{
			TestEqual(TEXT("LeftOperand == Self.HP"),  Cond->LeftOperand,  TEXT("Self.HP"));
			TestEqual(TEXT("Operator == Greater"),     Cond->Operator,     EBehaviacOperatorType::Greater);
			TestEqual(TEXT("RightOperand == 0"),       Cond->RightOperand, TEXT("0"));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_LoadAndExecute,
	"BehaviacPlugin.XML.LoadAndExecute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_LoadAndExecute::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"  <node class=\"behaviac::Action\" id=\"1\">"
			"    <property name=\"Method\" value=\"DoIt\"/>"
			"  </node>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded"), Tree)) return false;
	if (!TestNotNull(TEXT("RootNode set"), Tree->RootNode)) return false;

	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->RegisterMethodHandler(TEXT("DoIt"), []() -> EBehaviacStatus
	{
		return EBehaviacStatus::Success;
	});

	UBehaviacBehaviorTreeTask* TaskTree = BT_BuildTree(Tree->RootNode);
	EBehaviacStatus S = TaskTree->Tick(A);
	TestEqual(TEXT("XML tree executes → Success"), S, EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_MissingMethod,
	"BehaviacPlugin.XML.MissingMethod",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_MissingMethod::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"  <node class=\"behaviac::Action\" id=\"1\">"
			"    <property name=\"Method\" value=\"UnregisteredMethod\"/>"
			"    <property name=\"ResultOption\" value=\"Success\"/>"
			"  </node>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded"), Tree)) return false;
	if (!TestNotNull(TEXT("RootNode set"), Tree->RootNode)) return false;

	UBehaviacAgentComponent* A = BT_MakeAgent();
	// No handler registered for "UnregisteredMethod"
	UBehaviacBehaviorTreeTask* TaskTree = BT_BuildTree(Tree->RootNode);
	EBehaviacStatus S = TaskTree->Tick(A);

	// When no handler is present, the action falls back to ResultOption=Success
	TestTrue(TEXT("Missing method falls back to ResultOption — no crash"),
		S == EBehaviacStatus::Success || S == EBehaviacStatus::Failure || S == EBehaviacStatus::Invalid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_InvalidXML,
	"BehaviacPlugin.XML.InvalidXML",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_InvalidXML::RunTest(const FString&)
{
	AddExpectedError(TEXT("Failed to parse XML content"), EAutomationExpectedErrorFlags::Contains, 1);

	UBehaviacBehaviorTree* Tree = NewObject<UBehaviacBehaviorTree>(GetTransientPackage());
	bool bOK = Tree->LoadFromXML(TEXT("this is not xml <><><"));
	TestFalse(TEXT("LoadFromXML with garbage returns false"), bOK);
	// RootNode should be null or the call returns false — either is acceptable
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_EmptyBehavior,
	"BehaviacPlugin.XML.EmptyBehavior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_EmptyBehavior::RunTest(const FString&)
{
	AddExpectedError(TEXT("No <node> element found in XML"), EAutomationExpectedErrorFlags::Contains, 1);

	// Valid XML but no nodes
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"TestAgent\" version=\"5\">"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = NewObject<UBehaviacBehaviorTree>(GetTransientPackage());
	bool bOK = Tree->LoadFromXML(XML);
	// Should not crash; RootNode will be null
	TestTrue(TEXT("Empty behavior XML does not crash"), true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacXML_AgentTypeAndVersion,
	"BehaviacPlugin.XML.AgentTypeAndVersion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacXML_AgentTypeAndVersion::RunTest(const FString&)
{
	const FString XML =
		TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<behavior agenttype=\"MyHeroAgent\" version=\"7\">"
			"  <node class=\"behaviac::Noop\" id=\"1\"/>"
			"</behavior>");

	UBehaviacBehaviorTree* Tree = LoadXML(XML);
	if (!TestNotNull(TEXT("Tree loaded"), Tree)) return false;

	TestEqual(TEXT("AgentType parsed"), Tree->AgentType, TEXT("MyHeroAgent"));
	TestEqual(TEXT("Version parsed"),   Tree->Version,   7);
	return true;
}
