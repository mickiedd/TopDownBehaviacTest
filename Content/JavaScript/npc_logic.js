"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
//       All decision-making, timing, branching, and sequencing lives in the BT XML.
//
// Argv injected by UPuertsNPCComponent:
//   self     → ABehaviacAINPC (actor identity)
//   btBridge → UPuertsNPCComponent (BT dispatch + SetBTResult)
//   ai       → UJSAIInterface (all movement/sensor/state primitives)
const npcSelf = puerts.argv.getByName("self");
const btBridge = puerts.argv.getByName("btBridge");
const ai = puerts.argv.getByName("ai");
if (!npcSelf || !btBridge || !ai) {
    console.error(`[npc_logic] ERROR: missing argv — self:${!!npcSelf} btBridge:${!!btBridge} ai:${!!ai}`);
}
else {
    const name = String(npcSelf.GetName());
    console.log(`[npc_logic] ✅ Loaded for: ${name}`);
    const Running = 0;
    const Success = 1;
    const Failure = 2;
    // ── State machine ────────────────────────────────────────────────────────
    // UpdateAIState: the only place with logic — decides what state to enter.
    // Includes Panic and Taunt as valid states so the BT can branch into them.
    let lastTauntTime = 0;
    let panicEndTime = 0;
    const handlers = {
        "UpdateAIState": () => {
            const distFromPost = ai.GetDistanceFromPost();
            const distToPlayer = ai.GetDistanceToPlayer();
            const canSee = ai.CanSeePlayer();
            const current = String(ai.GetAIState());
            const t = Date.now();
            let next = "Patrol";
            if (canSee && distToPlayer <= ai.AttackRange) {
                next = "Combat";
                ai.SetLastKnownPos();
            }
            else if (canSee && distToPlayer <= ai.DetectionRadius) {
                ai.SetLastKnownPos();
                // 15% chance to panic when spotted close-range
                if (distToPlayer < 400 && Math.random() < 0.15 && t > panicEndTime) {
                    next = "Panic";
                    panicEndTime = t + 3000 + Math.random() * 3000;
                }
                else if (t < panicEndTime) {
                    next = "Panic"; // stay in panic until timer expires
                }
                else {
                    next = "Chase";
                }
            }
            else if (canSee && distToPlayer < 600 && t - lastTauntTime > 10000 && Math.random() < 0.2) {
                // Player is visible but far enough — taunt opportunity
                next = "Taunt";
                lastTauntTime = t;
            }
            else if (current === "Chase" || current === "Combat" || current === "Panic") {
                next = (distFromPost > ai.GuardRadius) ? "ReturnToPost" : "Investigate";
            }
            else if (current === "Taunt") {
                // Taunt completes naturally, go back to Chase or Patrol
                next = canSee ? "Chase" : "Patrol";
            }
            else if (distFromPost > ai.GuardRadius) {
                next = "ReturnToPost";
            }
            ai.SetAIState(next);
            return Success;
        },
        // ── Speed setters ────────────────────────────────────────────────────
        "SetWalkSpeed": () => { ai.SetSpeed(ai.WalkSpeed); return Success; },
        "SetRunSpeed": () => { ai.SetSpeed(ai.RunSpeed); return Success; },
        "SprintSpeed": () => { ai.SetSpeedRaw(ai.RunSpeed * 1.6); return Success; },
        // ── Patrol ───────────────────────────────────────────────────────────
        "Patrol": () => { ai.Patrol(); return Success; },
        "FindPlayer": () => {
            if (ai.CanSeePlayer()) {
                ai.SetLastKnownPos();
                return Success;
            }
            return Failure;
        },
        // ── Chase ────────────────────────────────────────────────────────────
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
        // Sprint burst: called by BT as optional decorator in Chase sequence.
        // Returns Success immediately (BT wraps in DecoratorAlwaysSuccess).
        "MaybeSprintBurst": () => {
            if (Math.random() < 0.2) {
                ai.SetSpeedRaw(ai.RunSpeed * 1.8);
                // Speed resets to RunSpeed on next SetRunSpeed call in the BT loop
            }
            return Success;
        },
        // ── Combat ───────────────────────────────────────────────────────────
        "AttackPlayer": () => {
            if (String(ai.GetAIState()) !== "Combat")
                return Failure;
            const dist = ai.GetDistanceToTarget();
            if (dist < 0 || dist > ai.CombatRange)
                return Failure;
            console.log(`[npc_logic][${name}] ⚔️ HIT! dist=${Math.round(dist)}`);
            return Success;
        },
        // Jump before attacking — BT wraps in DecoratorAlwaysSuccess
        "MaybeJumpAttack": () => {
            if (Math.random() < 0.3) {
                ai.LaunchUp(350);
            }
            return Success;
        },
        // ── Goofy one-shot actions (called directly by BT nodes) ─────────────
        "Jump": () => { ai.Jump(); return Success; },
        "Crouch": () => { ai.Crouch(); return Success; },
        "UnCrouch": () => { ai.UnCrouch(); return Success; },
        "Spin": () => { ai.Spin(180); return Success; },
        "RandomSpin": () => { ai.RandomSpin(); return Success; },
        "TauntJump": () => { ai.TauntJump(); return Success; },
        "FaceAwayFromPlayer": () => { ai.FaceAwayFromPlayer(); return Success; },
        "FleeFromPlayer": () => { ai.FleeFromPlayer(); return Running; },
        // MaybeCrouch: BT calls this at patrol start; wraps in DecoratorAlwaysSuccess
        "MaybeCrouch": () => {
            if (Math.random() < 0.3)
                ai.Crouch();
            return Success;
        },
        // ── Navigation ───────────────────────────────────────────────────────
        "MoveToTarget": () => {
            const dist = ai.GetDistanceToTarget();
            if (dist < 0)
                return Failure;
            if (dist <= ai.AttackRange)
                return Success;
            ai.MoveToTarget();
            return Running;
        },
        "MoveToLastKnownPos": () => ai.MoveToLastKnownPos() ? Success : Running,
        "ReturnToPost": () => {
            if (ai.GetDistanceFromPost() < 100) {
                ai.SetAIState("Patrol");
                return Success;
            }
            ai.MoveToPost();
            return Running;
        },
        // ── Utility ──────────────────────────────────────────────────────────
        "FaceTarget": () => { ai.FaceTarget(); return Success; },
        "StopMovement": () => { ai.StopMovement(); return Success; },
        "LookAround": () => { ai.LookAround(); return Success; },
        "ClearLastKnownPos": () => { ai.ClearLastKnownPos(); return Success; },
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
        // No handler → sentinel → C++ fallback
    });
    console.log(`[npc_logic] ✅ Handlers: [${Object.keys(handlers).join(", ")}]`);
    // ── Status logger ────────────────────────────────────────────────────────
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
