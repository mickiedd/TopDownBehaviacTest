// npc_logic.ts
// Puerts script attached to BP_AINPC via UPuertsNPCComponent.
// The owning NPC actor is injected as "self" via puerts.argv.
//
// What this demonstrates:
//  - Receiving a UObject reference from C++ via argv
//  - Reading UE properties/methods from TypeScript
//  - Periodic logic via setInterval
//  - Calling UE methods back from TS

import * as puerts from "puerts";

// ── 1. Grab the NPC actor passed from C++ ────────────────────────────────────
const self = puerts.argv.getByName("self") as any;

if (!self) {
    console.error("[npc_logic] ERROR: 'self' not found in argv — is PuertsNPCComponent attached?");
} else {
    const name: string = self.GetName();
    console.log(`[npc_logic] ✅ Script loaded for NPC: ${name}`);

    // ── 2. Read initial properties ───────────────────────────────────────────
    const detectionRadius: number = self.DetectionRadius;
    const walkSpeed: number       = self.WalkSpeed;
    const runSpeed: number        = self.RunSpeed;
    const guardRadius: number     = self.GuardRadius;

    console.log(`[npc_logic] 📋 Config — DetectionRadius: ${detectionRadius}, WalkSpeed: ${walkSpeed}, RunSpeed: ${runSpeed}, GuardRadius: ${guardRadius}`);

    // ── 3. Periodic status check every 3 seconds ─────────────────────────────
    let tickCount = 0;

    setInterval(() => {
        tickCount++;

        // Read current AI state from the Behaviac blackboard
        const aiState: string = self.GetBehaviacProperty("AIState") || "Unknown";

        // Read current world position
        const pos = self.GetActorLocation();
        const x = pos.X.toFixed(0);
        const y = pos.Y.toFixed(0);
        const z = pos.Z.toFixed(0);

        // Read velocity
        const vel = self.GetVelocity();
        const speed = Math.sqrt(vel.X * vel.X + vel.Y * vel.Y).toFixed(0);

        // Read target player (may be null)
        const target = self.TargetPlayer;
        const targetStr = target ? target.GetName() : "none";

        console.log(
            `[npc_logic][${name}] tick#${tickCount} | ` +
            `State: ${aiState} | ` +
            `Pos: (${x}, ${y}, ${z}) | ` +
            `Speed: ${speed} | ` +
            `Target: ${targetStr}`
        );

        // ── 4. Example: override AI behaviour from TS ────────────────────────
        // If we've been in Combat state for 5+ ticks, force a cooldown via TS
        if (aiState === "Combat" && tickCount % 5 === 0) {
            console.log(`[npc_logic][${name}] 🧠 TS override: forcing StopMovement for cooldown`);
            self.StopMovement();
        }

    }, 3000); // every 3 seconds

    console.log(`[npc_logic] ⏱️  Periodic status logger started (every 3s) for ${name}`);
}
