"use strict";
// npc_logic.js — compiled from TypeScript/npc_logic.ts
// Puerts script attached to BP_AINPC via UPuertsNPCComponent.

// puerts is a global in QuickJS backend — no require needed
const self = puerts.argv.getByName("self");

if (!self) {
    console.error("[npc_logic] ERROR: 'self' not found in argv — is PuertsNPCComponent attached?");
} else {
    const name = self.GetName();
    console.log(`[npc_logic] ✅ Script loaded for NPC: ${name}`);

    // 2. Read initial properties (UPROPERTY floats come back as JS numbers directly)
    console.log(`[npc_logic] 📋 Config — DetectionRadius: ${self.DetectionRadius}, WalkSpeed: ${self.WalkSpeed}, RunSpeed: ${self.RunSpeed}, GuardRadius: ${self.GuardRadius}`);

    // 3. Periodic status check every 3 seconds
    let tickCount = 0;

    setInterval(() => {
        try {
            tickCount++;

            // AIState from Behaviac blackboard
            const aiState = self.GetBehaviacProperty("AIState") || "Unknown";

            // World position — FVector fields come back as raw numbers in QuickJS
            const pos = self.GetActorLocation();
            const px = Math.round(+pos.X);
            const py = Math.round(+pos.Y);
            const pz = Math.round(+pos.Z);

            // Velocity
            const vel = self.GetVelocity();
            const speed = Math.round(Math.sqrt((+vel.X) * (+vel.X) + (+vel.Y) * (+vel.Y)));

            // Target player (may be null)
            const target = self.TargetPlayer;
            const targetStr = target ? target.GetName() : "none";

            console.log(
                `[npc_logic][${name}] tick#${tickCount} | ` +
                `State: ${aiState} | ` +
                `Pos: (${px}, ${py}, ${pz}) | ` +
                `Speed: ${speed} | ` +
                `Target: ${targetStr}`
            );

            // 4. Example override: StopMovement cooldown every 5 combat ticks
            if (aiState === "Combat" && tickCount % 5 === 0) {
                console.log(`[npc_logic][${name}] 🧠 TS override: forcing StopMovement for cooldown`);
                self.StopMovement();
            }

        } catch (e) {
            console.error(`[npc_logic][${name}] ❌ Tick error: ${e}`);
        }
    }, 3000);

    console.log(`[npc_logic] ⏱️  Periodic status logger started (every 3s) for ${name}`);
}
