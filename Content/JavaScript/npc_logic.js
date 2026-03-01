"use strict";
// npc_logic.js — compiled from TypeScript/npc_logic.ts
// Puerts script attached to BP_AINPC via UPuertsNPCComponent.

const puerts = require("puerts");

// 1. Grab the NPC actor passed from C++
const self = puerts.argv.getByName("self");

if (!self) {
    console.error("[npc_logic] ERROR: 'self' not found in argv — is PuertsNPCComponent attached?");
} else {
    const name = self.GetName();
    console.log(`[npc_logic] ✅ Script loaded for NPC: ${name}`);

    // 2. Read initial properties
    const detectionRadius = self.DetectionRadius;
    const walkSpeed       = self.WalkSpeed;
    const runSpeed        = self.RunSpeed;
    const guardRadius     = self.GuardRadius;

    console.log(`[npc_logic] 📋 Config — DetectionRadius: ${detectionRadius}, WalkSpeed: ${walkSpeed}, RunSpeed: ${runSpeed}, GuardRadius: ${guardRadius}`);

    // 3. Periodic status check every 3 seconds
    let tickCount = 0;

    setInterval(() => {
        tickCount++;

        const aiState = self.GetBehaviacProperty("AIState") || "Unknown";

        const pos = self.GetActorLocation();
        const x = pos.X.toFixed(0);
        const y = pos.Y.toFixed(0);
        const z = pos.Z.toFixed(0);

        const vel = self.GetVelocity();
        const speed = Math.sqrt(vel.X * vel.X + vel.Y * vel.Y).toFixed(0);

        const target = self.TargetPlayer;
        const targetStr = target ? target.GetName() : "none";

        console.log(
            `[npc_logic][${name}] tick#${tickCount} | ` +
            `State: ${aiState} | ` +
            `Pos: (${x}, ${y}, ${z}) | ` +
            `Speed: ${speed} | ` +
            `Target: ${targetStr}`
        );

        // 4. Example override: force StopMovement cooldown every 5 combat ticks
        if (aiState === "Combat" && tickCount % 5 === 0) {
            console.log(`[npc_logic][${name}] 🧠 TS override: forcing StopMovement for cooldown`);
            self.StopMovement();
        }

    }, 3000);

    console.log(`[npc_logic] ⏱️  Periodic status logger started (every 3s) for ${name}`);
}
