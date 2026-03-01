"use strict";
// npc_logic.js — Full behavior logic in TypeScript/JS
// All BT actions handled here; C++ provides only primitive movement/sensor helpers.

const self     = puerts.argv.getByName("self");
const btBridge = puerts.argv.getByName("btBridge");

if (!self || !btBridge) {
    console.error("[npc_logic] ERROR: missing argv (self or btBridge)");
} else {
    const name = String(self.GetName());
    console.log(`[npc_logic] ✅ Loaded for: ${name}`);
    console.log(`[npc_logic] 📋 DetectionRadius: ${self.DetectionRadius}, WalkSpeed: ${self.WalkSpeed}, RunSpeed: ${self.RunSpeed}, AttackRange: ${self.AttackRange}, CombatRange: ${self.CombatRange}, GuardRadius: ${self.GuardRadius}`);

    // ── BT result constants ──────────────────────────────────────────────────
    const Running = 0;
    const Success = 1;
    const Failure = 2;

    // ── Action handlers ──────────────────────────────────────────────────────
    const handlers = {

        // ── State machine ────────────────────────────────────────────────────
        "UpdateAIState": () => {
            const distFromPost   = self.JS_GetDistanceFromPost();
            const distToPlayer   = self.JS_GetDistanceToPlayer();
            const playerFromPost = self.JS_GetPlayerDistanceFromPost();
            const canSee         = self.JS_CanSeePlayer();
            const currentState   = String(self.GetBehaviacProperty("AIState"));

            let newState = "Patrol";

            if (canSee && distToPlayer <= self.AttackRange) {
                newState = "Combat";
                self.JS_SetLastKnownPos();
            } else if (canSee && distToPlayer <= self.DetectionRadius) {
                newState = "Chase";
                self.JS_SetLastKnownPos();
            } else if (currentState === "Chase" || currentState === "Combat") {
                // Lost sight — investigate last known position
                newState = (distFromPost > self.GuardRadius) ? "ReturnToPost" : "Investigate";
            } else if (distFromPost > self.GuardRadius) {
                newState = "ReturnToPost";
            }

            self.JS_SetAIState(newState);
            return Success;
        },

        // ── Movement speed ───────────────────────────────────────────────────
        "SetWalkSpeed": () => {
            self.JS_SetSpeed(self.WalkSpeed);
            return Success;
        },

        "SetRunSpeed": () => {
            self.JS_SetSpeed(self.RunSpeed);
            return Success;
        },

        // ── Patrol ───────────────────────────────────────────────────────────
        "Patrol": () => {
            self.JS_Patrol();
            return Running;
        },

        "FindPlayer": () => {
            const canSee = self.JS_CanSeePlayer();
            if (canSee) {
                self.JS_SetLastKnownPos();
                return Success;
            }
            return Failure;
        },

        "MoveToTarget": () => {
            const dist = self.GetDistanceToTarget();
            if (dist < 0) return Failure;
            if (dist <= self.AttackRange) return Success;
            self.JS_MoveToTarget();
            return Running;
        },

        // ── Chase ────────────────────────────────────────────────────────────
        "ChasePlayer": () => {
            const aiState = String(self.GetBehaviacProperty("AIState"));
            if (aiState !== "Chase") {
                self.JS_StopMovement();
                return Failure;
            }
            const dist = self.GetDistanceToTarget();
            if (dist < 0) return Failure;
            if (dist <= self.AttackRange) return Success;
            self.JS_MoveToTarget();
            return Running;
        },

        // ── Combat ───────────────────────────────────────────────────────────
        "AttackPlayer": () => {
            const aiState = String(self.GetBehaviacProperty("AIState"));
            if (aiState !== "Combat") return Failure;
            const dist = self.GetDistanceToTarget();
            if (dist < 0 || dist > self.CombatRange) return Failure;
            console.log(`[npc_logic][${name}] ⚔️ HIT! dist=${Math.round(dist)}`);
            return Success;
        },

        "FaceTarget": () => {
            self.JS_FaceTarget();
            return Success;
        },

        "StopMovement": () => {
            self.JS_StopMovement();
            return Success;
        },

        // ── Investigate ──────────────────────────────────────────────────────
        "MoveToLastKnownPos": () => {
            const arrived = self.JS_MoveToLastKnownPos();
            return arrived ? Success : Running;
        },

        "LookAround": () => {
            self.JS_LookAround();
            return Success;
        },

        "ClearLastKnownPos": () => {
            self.JS_ClearLastKnownPos();
            self.JS_SetAIState("Patrol");
            return Success;
        },

        // ── Return to post ───────────────────────────────────────────────────
        "ReturnToPost": () => {
            const dist = self.JS_GetDistanceFromPost();
            if (dist < 100) {
                self.JS_SetAIState("Patrol");
                return Success;
            }
            self.JS_SetSpeed(self.WalkSpeed);
            self.JS_MoveToPost();
            return Running;
        },
    };

    // ── Bind to BT dispatch delegate ─────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[String(actionName)];
        if (handler) {
            try {
                btBridge.SetBTResult(handler());
            } catch(e) {
                console.error(`[npc_logic][${name}] ❌ ${actionName}: ${e}`);
                btBridge.SetBTResult(Failure);
            }
        }
        // No handler → sentinel stays → C++ fallback
    });

    console.log(`[npc_logic] ✅ BT handlers registered: [${Object.keys(handlers).join(", ")}]`);

    // ── Status logger (every 3s) ─────────────────────────────────────────────
    let tick = 0;
    setInterval(() => {
        try {
            tick++;
            const state   = String(self.GetBehaviacProperty("AIState"));
            const px      = Math.round(self.GetLocationX());
            const py      = Math.round(self.GetLocationY());
            const speed   = Math.round(self.GetSpeedXY());
            const target  = self.TargetPlayer;
            const tStr    = target ? String(target.GetName()) : "none";
            console.log(`[npc_logic][${name}] tick#${tick} | ${state} | (${px},${py}) | spd:${speed} | tgt:${tStr}`);
        } catch(e) {}
    }, 3000);
}
