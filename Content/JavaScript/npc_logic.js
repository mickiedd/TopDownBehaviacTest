// npc_logic.ts — Full NPC behavior logic
// Compiled to Content/JavaScript/npc_logic.js by tsc.
//
// Argv injected by UPuertsNPCComponent:
//   npcSelf     → ABehaviacAINPC (actor identity)
//   btBridge → UPuertsNPCComponent (BT dispatch delegate + SetBTResult)
//   ai       → UJSAIInterface (all movement/sensor/state primitives)
const npcSelf = puerts.argv.getByName("npcSelf");
const btBridge = puerts.argv.getByName("btBridge");
const ai = puerts.argv.getByName("ai");
if (!npcSelf || !btBridge || !ai) {
    console.error(`[npc_logic] ERROR: missing argv — npcSelf:${!!npcSelf} btBridge:${!!btBridge} ai:${!!ai}`);
}
else {
    const name = String(npcSelf.GetName());
    console.log(`[npc_logic] ✅ Loaded for: ${name}`);
    console.log(`[npc_logic] 📋 DetectionRadius:${ai.DetectionRadius} WalkSpeed:${ai.WalkSpeed} RunSpeed:${ai.RunSpeed} AttackRange:${ai.AttackRange} CombatRange:${ai.CombatRange} GuardRadius:${ai.GuardRadius}`);
    // ── BT result constants ──────────────────────────────────────────────────
    const Running = 0;
    const Success = 1;
    const Failure = 2;
    // ── Action handlers ──────────────────────────────────────────────────────
    const handlers = {
        "UpdateAIState": () => {
            const distFromPost = ai.GetDistanceFromPost();
            const distToPlayer = ai.GetDistanceToPlayer();
            const canSee = ai.CanSeePlayer();
            const currentState = String(ai.GetAIState());
            let newState = "Patrol";
            if (canSee && distToPlayer <= ai.AttackRange) {
                newState = "Combat";
                ai.SetLastKnownPos();
            }
            else if (canSee && distToPlayer <= ai.DetectionRadius) {
                newState = "Chase";
                ai.SetLastKnownPos();
            }
            else if (currentState === "Chase" || currentState === "Combat") {
                newState = (distFromPost > ai.GuardRadius) ? "ReturnToPost" : "Investigate";
            }
            else if (distFromPost > ai.GuardRadius) {
                newState = "ReturnToPost";
            }
            ai.SetAIState(newState);
            return Success;
        },
        "SetWalkSpeed": () => { ai.SetSpeed(ai.WalkSpeed); return Success; },
        "SetRunSpeed": () => { ai.SetSpeed(ai.RunSpeed); return Success; },
        "FindPlayer": () => {
            if (ai.CanSeePlayer()) {
                ai.SetLastKnownPos();
                return Success;
            }
            return Failure;
        },
        "Patrol": () => { ai.Patrol(); return Running; },
        "MoveToTarget": () => {
            const dist = ai.GetDistanceToTarget();
            if (dist < 0)
                return Failure;
            if (dist <= ai.AttackRange)
                return Success;
            ai.MoveToTarget();
            return Running;
        },
        "ChasePlayer": () => {
            if (String(ai.GetAIState()) !== "Chase") {
                ai.StopMovement();
                return Failure;
            }
            const dist = ai.GetDistanceToTarget();
            if (dist < 0)
                return Failure;
            if (dist <= ai.AttackRange)
                return Success;
            ai.MoveToTarget();
            return Running;
        },
        "AttackPlayer": () => {
            if (String(ai.GetAIState()) !== "Combat")
                return Failure;
            const dist = ai.GetDistanceToTarget();
            if (dist < 0 || dist > ai.CombatRange)
                return Failure;
            console.log(`[npc_logic][${name}] ⚔️ HIT! dist=${Math.round(dist)}`);
            return Success;
        },
        "FaceTarget": () => { ai.FaceTarget(); return Success; },
        "StopMovement": () => { ai.StopMovement(); return Success; },
        "MoveToLastKnownPos": () => ai.MoveToLastKnownPos() ? Success : Running,
        "LookAround": () => { ai.LookAround(); return Success; },
        "ClearLastKnownPos": () => {
            ai.ClearLastKnownPos();
            ai.SetAIState("Patrol");
            return Success;
        },
        "ReturnToPost": () => {
            if (ai.GetDistanceFromPost() < 100) {
                ai.SetAIState("Patrol");
                return Success;
            }
            ai.SetSpeed(ai.WalkSpeed);
            ai.MoveToPost();
            return Running;
        },
    };
    // ── Bind to BT dispatch delegate ─────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[String(actionName)];
        if (handler) {
            try {
                btBridge.SetBTResult(handler());
            }
            catch (e) {
                console.error(`[npc_logic][${name}] ❌ ${actionName}: ${e}`);
                btBridge.SetBTResult(Failure);
            }
        }
        // No handler → SetBTResult not called → sentinel preserved → C++ fallback
    });
    console.log(`[npc_logic] ✅ Handlers: [${Object.keys(handlers).join(", ")}]`);
    // ── Status logger (every 3s) ─────────────────────────────────────────────
    let tick = 0;
    setInterval(() => {
        try {
            tick++;
            const state = String(ai.GetAIState());
            const px = Math.round(ai.GetLocationX());
            const py = Math.round(ai.GetLocationY());
            const speed = Math.round(ai.GetSpeedXY());
            const tStr = ai.TargetActor ? String(ai.TargetActor.GetName()) : "none";
            console.log(`[npc_logic][${name}] #${tick} | ${state} | (${px},${py}) | spd:${speed} | tgt:${tStr}`);
        }
        catch (e) { /* swallow */ }
    }, 3000);
}
