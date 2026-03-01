"use strict";
// npc_logic.js — TS/JS logic layer for BP_AINPC
// BT actions are dispatched here from C++ via PuertsNPCComponent.OnBTAction

const self      = puerts.argv.getByName("self");
const btBridge  = puerts.argv.getByName("btBridge");

if (!self || !btBridge) {
    console.error("[npc_logic] ERROR: missing argv (self or btBridge)");
} else {
    const name = String(self.GetName());
    console.log(`[npc_logic] ✅ Loaded for NPC: ${name}`);
    console.log(`[npc_logic] 📋 DetectionRadius: ${self.DetectionRadius}, WalkSpeed: ${self.WalkSpeed}, RunSpeed: ${self.RunSpeed}, GuardRadius: ${self.GuardRadius}`);

    // ── BT result constants ──────────────────────────────────────────────────
    const Running = 0;
    const Success = 1;
    const Failure = 2;

    // ── Action handler map ───────────────────────────────────────────────────
    // Return Running/Success/Failure. Call btBridge.SetBTResult(n) then return.
    // Any action NOT listed here falls back to the C++ implementation.
    const handlers = {

        "ChasePlayer": () => {
            const aiState = String(self.GetBehaviacProperty("AIState"));
            if (aiState !== "Chase") {
                console.log(`[npc_logic][${name}] ChasePlayer: state is ${aiState}, bailing`);
                return Failure;
            }
            const target = self.TargetPlayer;
            if (!target) {
                console.log(`[npc_logic][${name}] ChasePlayer: no target`);
                return Failure;
            }
            const dist = Math.round(self.GetDistanceToTarget());
            const attackRange = self.AttackRange || 150;
            if (dist <= attackRange) {
                console.log(`[npc_logic][${name}] ChasePlayer: in attack range (${dist})`);
                return Success;
            }
            console.log(`[npc_logic][${name}] ChasePlayer: sprinting (dist=${dist})`);
            self.JS_MoveToTarget();   // C++ primitive: MoveToActor
            return Running;
        },

        "AttackPlayer": () => {
            const aiState = String(self.GetBehaviacProperty("AIState"));
            if (aiState !== "Combat") return Failure;
            const target = self.TargetPlayer;
            if (!target) return Failure;
            const dist = Math.round(self.GetDistanceToTarget());
            const combatRange = self.CombatRange || 200;
            if (dist > combatRange) {
                console.log(`[npc_logic][${name}] AttackPlayer: out of range (${dist})`);
                return Failure;
            }
            console.log(`[npc_logic][${name}] ⚔️ AttackPlayer: HIT! dist=${dist}`);
            return Success;
        },

        "Patrol": () => {
            console.log(`[npc_logic][${name}] Patrol: JS patrolling`);
            self.JS_Patrol();    // C++ primitive: move to next patrol point
            return Running;
        },

        "StopMovement": () => {
            self.JS_StopMovement();
            return Success;
        },

        "LookAround": () => {
            self.JS_LookAround();
            return Success;
        },
    };

    // ── Bind to BT dispatch delegate ─────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[String(actionName)];
        if (handler) {
            try {
                const result = handler();
                btBridge.SetBTResult(result);
            } catch(e) {
                console.error(`[npc_logic][${name}] ❌ Handler error for ${actionName}: ${e}`);
                btBridge.SetBTResult(2); // Failure
            }
        }
        // No handler → do NOT call SetBTResult → sentinel stays → C++ fallback runs
    });

    console.log(`[npc_logic] ✅ BT handler bound. Handlers: [${Object.keys(handlers).join(", ")}]`);
    console.log(`[npc_logic] 📋 Unregistered actions fall back to C++ implementations.`);

    // ── Status logger ────────────────────────────────────────────────────────
    let tickCount = 0;
    setInterval(() => {
        try {
            tickCount++;
            const aiState = String(self.GetBehaviacProperty("AIState"));
            const px = Math.round(self.GetLocationX());
            const py = Math.round(self.GetLocationY());
            const speed = Math.round(self.GetSpeedXY());
            const target = self.TargetPlayer;
            const targetStr = target ? String(target.GetName()) : "none";
            console.log(`[npc_logic][${name}] tick#${tickCount} | State: ${aiState} | Pos: (${px}, ${py}) | Speed: ${speed} | Target: ${targetStr}`);
        } catch(e) {
            console.error(`[npc_logic][${name}] ❌ Status tick error: ${e}`);
        }
    }, 3000);
}
