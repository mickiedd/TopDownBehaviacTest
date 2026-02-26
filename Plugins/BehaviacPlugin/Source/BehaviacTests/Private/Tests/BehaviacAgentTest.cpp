// Behaviac UE5 Plugin — Agent Component Tests
// Licensed under the BSD 3-Clause License.
//
// Run via Session Frontend: Automation > BehaviacPlugin.Agent.*
// Console: Automation RunTests BehaviacPlugin.Agent

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ---------------------------------------------------------------------------
// Property system
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_PropertySetGet,
	"BehaviacPlugin.Agent.PropertySetGet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_PropertySetGet::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetPropertyValue(TEXT("Name"), TEXT("Alice"));
	TestEqual(TEXT("String round-trip"), A->GetPropertyValue(TEXT("Name")), TEXT("Alice"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_PropertyInt,
	"BehaviacPlugin.Agent.PropertyInt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_PropertyInt::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetIntProperty(TEXT("HP"), 99);
	TestEqual(TEXT("Int round-trip"), A->GetIntProperty(TEXT("HP")), 99);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_PropertyFloat,
	"BehaviacPlugin.Agent.PropertyFloat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_PropertyFloat::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetFloatProperty(TEXT("Speed"), 3.14f);
	TestTrue(TEXT("Float round-trip"), FMath::IsNearlyEqual(A->GetFloatProperty(TEXT("Speed")), 3.14f, 0.001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_PropertyBool,
	"BehaviacPlugin.Agent.PropertyBool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_PropertyBool::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SetBoolProperty(TEXT("IsAlive"), true);
	TestTrue(TEXT("Bool true round-trip"), A->GetBoolProperty(TEXT("IsAlive")));
	A->SetBoolProperty(TEXT("IsAlive"), false);
	TestFalse(TEXT("Bool false round-trip"), A->GetBoolProperty(TEXT("IsAlive")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_HasProperty,
	"BehaviacPlugin.Agent.PropertyHasProperty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_HasProperty::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestFalse(TEXT("Not present before set"), A->HasProperty(TEXT("X")));
	A->SetPropertyValue(TEXT("X"), TEXT("1"));
	TestTrue(TEXT("Present after set"), A->HasProperty(TEXT("X")));
	return true;
}

// ---------------------------------------------------------------------------
// Signal system
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_SignalSetClearCheck,
	"BehaviacPlugin.Agent.SignalSetClearCheck",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_SignalSetClearCheck::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	TestFalse(TEXT("Signal not set initially"), A->IsSignalSet(TEXT("Fire")));
	A->SendSignal(TEXT("Fire"));
	TestTrue(TEXT("Signal set after Send"), A->IsSignalSet(TEXT("Fire")));
	A->ClearSignal(TEXT("Fire"));
	TestFalse(TEXT("Signal cleared"), A->IsSignalSet(TEXT("Fire")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_ClearAllSignals,
	"BehaviacPlugin.Agent.ClearAllSignals",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_ClearAllSignals::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->SendSignal(TEXT("A"));
	A->SendSignal(TEXT("B"));
	A->SendSignal(TEXT("C"));
	A->ClearAllSignals();
	TestFalse(TEXT("A cleared"), A->IsSignalSet(TEXT("A")));
	TestFalse(TEXT("B cleared"), A->IsSignalSet(TEXT("B")));
	TestFalse(TEXT("C cleared"), A->IsSignalSet(TEXT("C")));
	return true;
}

// ---------------------------------------------------------------------------
// Method dispatch
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_MethodHandler,
	"BehaviacPlugin.Agent.MethodHandler",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_MethodHandler::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->RegisterMethodHandler(TEXT("Attack"), []() -> EBehaviacStatus
	{
		return EBehaviacStatus::Success;
	});
	EBehaviacStatus Result = A->ExecuteMethod(TEXT("Attack"));
	TestEqual(TEXT("Registered handler returns Success"),
		Result, EBehaviacStatus::Success);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_MethodHandlerRunning,
	"BehaviacPlugin.Agent.MethodHandlerRunning",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_MethodHandlerRunning::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	A->RegisterMethodHandler(TEXT("LongAction"), []() -> EBehaviacStatus
	{
		return EBehaviacStatus::Running;
	});
	TestEqual(TEXT("Handler returning Running"), A->ExecuteMethod(TEXT("LongAction")),
		EBehaviacStatus::Running);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacAgent_MethodHandlerUnknown,
	"BehaviacPlugin.Agent.MethodHandlerUnknown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacAgent_MethodHandlerUnknown::RunTest(const FString&)
{
	UBehaviacAgentComponent* A = BT_MakeAgent();
	// No handler registered — should return Invalid (not crash)
	EBehaviacStatus Result = A->ExecuteMethod(TEXT("NonExistentMethod"));
	TestEqual(TEXT("Unregistered method returns Invalid"),
		Result, EBehaviacStatus::Invalid);
	return true;
}
