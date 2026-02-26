// Behaviac UE5 Plugin — Parallel + SelectorLoop Interrupt Tests
// Licensed under the BSD 3-Clause License.
//
// Run via: Automation RunTests BehaviacPlugin.ParallelInterrupt
//
// These tests isolate the exact production bug:
//   Parallel(ChildFinishPolicy=Loop) should re-tick child[0] (UpdateAIState)
//   every frame; SelectorLoop should interrupt its active branch when a
//   higher-priority condition becomes true.

#include "Misc/AutomationTest.h"
#include "BehaviacTestHelpers.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/** Build a Condition node that reads Self.<Key> and compares to Value. */
static UBehaviacCondition* MakePropertyCondition(
	const FString& Key,
	const FString& Value)
{
	return BT_MakeCondition(
		FString::Printf(TEXT("Self.%s"), *Key),
		EBehaviacOperatorType::Equal,
		Value);
}

/** Build a WithPrecondition(Condition(key==val), Body). */
static UBehaviacWithPrecondition* MakeGuardedBranch(
	const FString& Key,
	const FString& Value,
	UBehaviacBehaviorNode* Body)
{
	UBehaviacWithPrecondition* WP = NewObject<UBehaviacWithPrecondition>(GetTransientPackage());
	WP->AddChild(MakePropertyCondition(Key, Value));
	WP->AddChild(Body);
	return WP;
}

/** Build a DecoratorLoop(Count=-1) around a child node. */
static UBehaviacDecoratorLoop* MakeInfiniteLoop(UBehaviacBehaviorNode* Child)
{
	UBehaviacDecoratorLoop* Loop = NewObject<UBehaviacDecoratorLoop>(GetTransientPackage());
	Loop->LoopCount = -1;
	Loop->AddChild(Child);
	return Loop;
}

// ===========================================================================
// TEST 1 — Parallel ChildFinishPolicy::Loop keeps re-ticking child[0]
//
// Tree:
//   Parallel(Loop)
//     [0] Action("Updater")   ← returns Success each tick
//     [1] DecoratorLoop(-1) → Action("Worker") ← returns Running forever
//
// Expected: Updater is called on EVERY tick (not just tick 1).
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacParallelInterrupt_ParallelLoopReticks,
	"BehaviacPlugin.ParallelInterrupt.Parallel_LoopPolicy_RetickChild0",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacParallelInterrupt_ParallelLoopReticks::RunTest(const FString&)
{
	UBehaviacAgentComponent* Agent = BT_MakeAgent();

	int32 UpdaterCount = 0;
	int32 WorkerCount  = 0;

	Agent->RegisterMethodHandler(TEXT("Updater"), [&]() -> EBehaviacStatus {
		UpdaterCount++;
		return EBehaviacStatus::Success;
	});
	Agent->RegisterMethodHandler(TEXT("Worker"), [&]() -> EBehaviacStatus {
		WorkerCount++;
		return EBehaviacStatus::Running;
	});

	UBehaviacAction* UpdaterNode = NewObject<UBehaviacAction>(GetTransientPackage());
	UpdaterNode->MethodName = TEXT("Updater");

	UBehaviacAction* WorkerNode = NewObject<UBehaviacAction>(GetTransientPackage());
	WorkerNode->MethodName = TEXT("Worker");

	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy    = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Loop;   // KEY: must re-tick child[0]
	Par->AddChild(NewObject<UBehaviacDecoratorAlwaysSuccess>(GetTransientPackage())); // placeholder
	Par->AddChild(MakeInfiniteLoop(WorkerNode));

	// Replace child[0] with real updater
	Par->Children[0] = UpdaterNode;

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Par);

	const int32 Ticks = 5;
	BT_TickN(Tree, Agent, Ticks);

	UE_LOG(LogTemp, Warning, TEXT("[Test] ParallelLoop: UpdaterCount=%d WorkerCount=%d (ticks=%d)"),
		UpdaterCount, WorkerCount, Ticks);

	TestEqual(TEXT("Updater called every tick (ChildFinishPolicy=Loop)"), UpdaterCount, Ticks);
	TestTrue (TEXT("Worker called at least once"), WorkerCount >= 1);
	return true;
}

// ===========================================================================
// TEST 2 — Parallel ChildFinishPolicy::Once only ticks child[0] once
//
// Sanity check that Once behaves differently.
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacParallelInterrupt_ParallelOnceSkipsChild0,
	"BehaviacPlugin.ParallelInterrupt.Parallel_OncePolicy_SkipsChild0AfterFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacParallelInterrupt_ParallelOnceSkipsChild0::RunTest(const FString&)
{
	UBehaviacAgentComponent* Agent = BT_MakeAgent();

	int32 UpdaterCount = 0;
	int32 WorkerCount  = 0;

	Agent->RegisterMethodHandler(TEXT("Updater2"), [&]() -> EBehaviacStatus {
		UpdaterCount++;
		return EBehaviacStatus::Success;
	});
	Agent->RegisterMethodHandler(TEXT("Worker2"), [&]() -> EBehaviacStatus {
		WorkerCount++;
		return EBehaviacStatus::Running;
	});

	UBehaviacAction* UpdaterNode = NewObject<UBehaviacAction>(GetTransientPackage());
	UpdaterNode->MethodName = TEXT("Updater2");

	UBehaviacAction* WorkerNode = NewObject<UBehaviacAction>(GetTransientPackage());
	WorkerNode->MethodName = TEXT("Worker2");

	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy     = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Once;  // Should cache child[0] after tick 1
	Par->AddChild(UpdaterNode);
	Par->AddChild(MakeInfiniteLoop(WorkerNode));

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Par);
	BT_TickN(Tree, Agent, 5);

	UE_LOG(LogTemp, Warning, TEXT("[Test] ParallelOnce: UpdaterCount=%d (expected 1)"), UpdaterCount);

	TestEqual(TEXT("Once policy: Updater called exactly once"), UpdaterCount, 1);
	return true;
}

// ===========================================================================
// TEST 3 — SelectorLoop interrupts lower-priority branch when blackboard changes
//
// Tree:
//   SelectorLoop
//     [0] WithPrecondition(AIState=="Chase")  → Action("ChaseAction")
//     [1] Sequence → Action("PatrolAction")   ← fallthrough
//
// Tick 1-3:  AIState="Patrol" → ChaseAction never called, PatrolAction runs
// Tick 4:    set AIState="Chase" → SelectorLoop should switch to Chase branch
// Tick 5-6:  ChaseAction should be called, PatrolAction should NOT
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacParallelInterrupt_SelectorLoopInterrupts,
	"BehaviacPlugin.ParallelInterrupt.SelectorLoop_InterruptsOnStateChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacParallelInterrupt_SelectorLoopInterrupts::RunTest(const FString&)
{
	UBehaviacAgentComponent* Agent = BT_MakeAgent();

	int32 ChaseCount  = 0;
	int32 PatrolCount = 0;
	int32 TickNum     = 0;

	Agent->SetPropertyValue(TEXT("AIState"), TEXT("Patrol"));

	Agent->RegisterMethodHandler(TEXT("ChaseAction"), [&]() -> EBehaviacStatus {
		ChaseCount++;
		return EBehaviacStatus::Running;
	});
	Agent->RegisterMethodHandler(TEXT("PatrolAction"), [&]() -> EBehaviacStatus {
		PatrolCount++;
		// On tick 4, flip state so SelectorLoop should interrupt next tick
		if (TickNum == 3) // 0-indexed: 4th tick
		{
			Agent->SetPropertyValue(TEXT("AIState"), TEXT("Chase"));
			UE_LOG(LogTemp, Warning, TEXT("[Test] ← AIState flipped to Chase on tick %d"), TickNum+1);
		}
		return EBehaviacStatus::Running;
	});

	UBehaviacAction* ChaseNode = NewObject<UBehaviacAction>(GetTransientPackage());
	ChaseNode->MethodName = TEXT("ChaseAction");

	UBehaviacAction* PatrolNode = NewObject<UBehaviacAction>(GetTransientPackage());
	PatrolNode->MethodName = TEXT("PatrolAction");

	// Build: SelectorLoop → [WithPrecondition(Chase), Sequence(Patrol-loop)]
	UBehaviacSelectorLoop* SL = NewObject<UBehaviacSelectorLoop>(GetTransientPackage());
	SL->AddChild(MakeGuardedBranch(TEXT("AIState"), TEXT("Chase"), MakeInfiniteLoop(ChaseNode)));
	SL->AddChild(BT_MakeSequence({ MakeInfiniteLoop(PatrolNode) }));

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(SL);

	const int32 TotalTicks = 6;
	for (int32 i = 0; i < TotalTicks; i++)
	{
		TickNum = i;
		Tree->Tick(Agent);
		UE_LOG(LogTemp, Warning,
			TEXT("[Test] Tick %d — AIState=%s — Chase=%d Patrol=%d"),
			i+1, *Agent->GetPropertyValue(TEXT("AIState")), ChaseCount, PatrolCount);
	}

	// Ticks 1-4: patrol ran (state was "Patrol")
	// Ticks 5-6: chase ran (state flipped to "Chase" at end of tick 4)
	TestTrue(TEXT("PatrolAction ran before state change"), PatrolCount >= 3);
	TestTrue(TEXT("ChaseAction ran after state change"),   ChaseCount  >= 1);
	TestEqual(TEXT("PatrolAction stopped after state change"), PatrolCount, 4);
	TestEqual(TEXT("ChaseAction ran ticks 5+6"), ChaseCount, 2);
	return true;
}

// ===========================================================================
// TEST 4 — Full simulation: Parallel(Loop) + SelectorLoop interrupt
//
// This replicates the exact production tree structure:
//   Parallel(Loop)
//     [0] DecoratorAlwaysSuccess → Action("UpdateState")  [writes AIState]
//     [1] DecoratorLoop(-1) → SelectorLoop
//           WithPrecondition(AIState=="Chase") → DecoratorLoop(-1) → Action("Chase")
//           Sequence → DecoratorLoop(-1) → Action("Patrol")  [fallthrough]
//
// Phase 1 (ticks 1-3):  UpdateState writes "Patrol" → Patrol runs
// Phase 2 (tick 4):     UpdateState writes "Chase"  → SelectorLoop should interrupt
// Phase 3 (ticks 5-6):  Chase runs, Patrol does NOT
// ===========================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBehaviacParallelInterrupt_FullSimulation,
	"BehaviacPlugin.ParallelInterrupt.FullSimulation_ParallelDrivesSelector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FBehaviacParallelInterrupt_FullSimulation::RunTest(const FString&)
{
	UBehaviacAgentComponent* Agent = BT_MakeAgent();

	int32 UpdateCount  = 0;
	int32 ChaseCount   = 0;
	int32 PatrolCount  = 0;
	int32 TickNum      = 0;

	Agent->SetPropertyValue(TEXT("AIState"), TEXT("Patrol"));

	// UpdateState: returns Success, flips to Chase on tick 4
	Agent->RegisterMethodHandler(TEXT("FS_UpdateState"), [&]() -> EBehaviacStatus {
		UpdateCount++;
		if (TickNum >= 3)
		{
			Agent->SetPropertyValue(TEXT("AIState"), TEXT("Chase"));
		}
		return EBehaviacStatus::Success;
	});

	Agent->RegisterMethodHandler(TEXT("FS_Chase"), [&]() -> EBehaviacStatus {
		ChaseCount++;
		return EBehaviacStatus::Running;
	});

	Agent->RegisterMethodHandler(TEXT("FS_Patrol"), [&]() -> EBehaviacStatus {
		PatrolCount++;
		return EBehaviacStatus::Running;
	});

	auto MakeActionNode = [](const FString& Name) -> UBehaviacAction* {
		UBehaviacAction* N = NewObject<UBehaviacAction>(GetTransientPackage());
		N->MethodName = Name;
		return N;
	};

	// Build SelectorLoop
	UBehaviacSelectorLoop* SL = NewObject<UBehaviacSelectorLoop>(GetTransientPackage());
	SL->AddChild(MakeGuardedBranch(
		TEXT("AIState"), TEXT("Chase"),
		MakeInfiniteLoop(MakeActionNode(TEXT("FS_Chase")))));
	SL->AddChild(BT_MakeSequence({
		MakeInfiniteLoop(MakeActionNode(TEXT("FS_Patrol")))
	}));

	// Wrap SelectorLoop in DecoratorLoop(-1)
	UBehaviacDecoratorLoop* SLLoop = MakeInfiniteLoop(SL);

	// Build Parallel(Loop)
	UBehaviacDecoratorAlwaysSuccess* UpdateWrapper = NewObject<UBehaviacDecoratorAlwaysSuccess>(GetTransientPackage());
	UpdateWrapper->AddChild(MakeActionNode(TEXT("FS_UpdateState")));

	UBehaviacParallel* Par = NewObject<UBehaviacParallel>(GetTransientPackage());
	Par->FailurePolicy     = EBehaviacParallelPolicy::FailOnOne_SucceedOnAll;
	Par->ChildFinishPolicy = EBehaviacChildFinishPolicy::Loop;
	Par->AddChild(UpdateWrapper);
	Par->AddChild(SLLoop);

	UBehaviacBehaviorTreeTask* Tree = BT_BuildTree(Par);

	const int32 TotalTicks = 6;
	for (int32 i = 0; i < TotalTicks; i++)
	{
		TickNum = i;
		Tree->Tick(Agent);
		UE_LOG(LogTemp, Warning,
			TEXT("[Test] Tick %d — AIState=%s — Update=%d Chase=%d Patrol=%d"),
			i+1, *Agent->GetPropertyValue(TEXT("AIState")),
			UpdateCount, ChaseCount, PatrolCount);
	}

	// UpdateState must run every tick
	TestEqual(TEXT("UpdateState called every tick"), UpdateCount, TotalTicks);
	// Patrol ran for ticks 1-3
	TestTrue(TEXT("Patrol ran during Patrol phase"), PatrolCount >= 3);
	// Chase ran for ticks 5-6 (after state flipped on tick 4)
	TestTrue(TEXT("Chase ran after state flip"), ChaseCount >= 1);
	// Patrol did NOT run during Chase phase
	TestEqual(TEXT("Patrol stopped after Chase state set"), PatrolCount, 3);

	return true;
}
