"use strict";
// npc_logic.js — Full behavior logic in TypeScript/JS
// All BT actions handled here via UJSAIInterface (ai) + UPuertsNPCComponent (btBridge).

const self     = puerts.argv.getByName("self");
const btBridge = puerts.argv.getByName("btBridge");
const ai       = puerts.argv.getByName("ai");

if (!self || !btBridge || !ai) {
    console.error(`[npc_logic] ERROR: missing argv — self:${!!self} btBridge:${!!btBridge} ai:${!!ai}`);
} else {
    const name = String(self.GetName());
    console.log(`[npc_logic] ✅ Loaded for: ${name}`);
    console.log(`[npc_logic] 📋 DetectionRadius:${ai.DetectionRadius} WalkSpeed:${ai.WalkSpeed} RunSpeed:${ai.RunSpeed} AttackRange:${ai.AttackRange} CombatRange:${ai.CombatRange} GuardRadius:${ai.GuardRadius}`);

    const Running = 0;
    const Success = 1;
    const Failure = 2;

    const handlers = {

        "UpdateAIState": () => {
            const distFromPost = ai.GetDistanceFromPost();
            const distToPlayer = ai.GetDistanceToPlayer();
            const canSee       = ai.CanSeePlayer();
            const currentState = String(ai.GetAIState());

            let newState = "Patrol";

            if (canSee && distToPlayer <= ai.AttackRange) {
                newState = "Combat";
                ai.SetLastKnownPos();
            } else if (canSee && distToPlayer <= ai.DetectionRadius) {
                newState = "Chase";
                ai.SetLastKnownPos();
            } else if (currentState === "Chase" || currentState === "Combat") {
                newState = (distFromPost > ai.GuardRadius) ? "ReturnToPost" : "Investigate";
            } else if (distFromPost > ai.GuardRadius) {
                newState = "ReturnToPost";
            }

            ai.SetAIState(newState);
            return Success;
        },

        "SetWalkSpeed": () => { ai.SetSpeed(ai.WalkSpeed); return Success; },
        "SetRunSpeed":  () => { ai.SetSpeed(ai.RunSpeed);  return Success; },

        "FindPlayer": () => {
            if (ai.CanSeePlayer()) { ai.SetLastKnownPos(); return Success; }
            return Failure;
        },

        "Patrol": () => { ai.Patrol(); return Running; },

        "MoveToTarget": () => {
            const dist = ai.GetDistanceToTarget();
            if (dist < 0) return Failure;
            if (dist <= ai.AttackRange) return Success;
            ai.MoveToTarget();
            return Running;
        },

        "ChasePlayer": () => {
            if (String(ai.GetAIState()) !== "Chase") { ai.StopMovement(); return Failure; }
            const dist = ai.GetDistanceToTarget();
            if (dist < 0) return Failure;
            if (dist <= ai.AttackRange) return Success;
            ai.MoveToTarget();
            return Running;
        },

        "AttackPlayer": () => {
            if (String(ai.GetAIState()) !== "Combat") return Failure;
            const dist = ai.GetDistanceToTarget();
            if (dist < 0 || dist > ai.CombatRange) return Failure;
            console.log(`[npc_logic][${name}] ⚔️ HIT! dist=${Math.round(dist)}`);
            return Success;
        },

        "FaceTarget":   () => { ai.FaceTarget();   return Success; },
        "StopMovement": () => { ai.StopMovement(); return Success; },

        "MoveToLastKnownPos": () => ai.MoveToLastKnownPos() ? Success : Running,

        "LookAround": () => { ai.LookAround(); return Success; },

        "ClearLastKnownPos": () => {
            ai.ClearLastKnownPos();
            ai.SetAIState("Patrol");
            return Success;
        },

        "ReturnToPost": () => {
            if (ai.GetDistanceFromPost() < 100) { ai.SetAIState("Patrol"); return Success; }
            ai.SetSpeed(ai.WalkSpeed);
            ai.MoveToPost();
            return Running;
        },
    };

    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[String(actionName)];
        if (handler) {
            try { btBridge.SetBTResult(handler()); }
            catch(e) {
                console.error(`[npc_logic][${name}] ❌ ${actionName}: ${e}`);
                btBridge.SetBTResult(Failure);
            }
        }
    });

    console.log(`[npc_logic] ✅ Handlers: [${Object.keys(handlers).join(", ")}]`);

    // Status logger
    let tick = 0;
    setInterval(() => {
        try {
            tick++;
            const state  = String(ai.GetAIState());
            const px     = Math.round(ai.GetLocationX());
            const py     = Math.round(ai.GetLocationY());
            const speed  = Math.round(ai.GetSpeedXY());
            const target = ai.TargetActor;
            const tStr   = target ? String(target.GetName()) : "none";
            console.log(`[npc_logic][${name}] #${tick} | ${state} | (${px},${py}) | spd:${speed} | tgt:${tStr}`);
        } catch(e) {}
    }, 3000);
}
