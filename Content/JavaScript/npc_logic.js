"use strict";
// npc_logic.js — compiled from TypeScript/npc_logic.ts

const self = puerts.argv.getByName("self");

if (!self) {
    console.error("[npc_logic] ERROR: 'self' not found in argv");
} else {
    const name = String(self.GetName());
    console.log(`[npc_logic] ✅ Script loaded for NPC: ${name}`);
    console.log(`[npc_logic] 📋 Config — DetectionRadius: ${self.DetectionRadius}, WalkSpeed: ${self.WalkSpeed}, RunSpeed: ${self.RunSpeed}, GuardRadius: ${self.GuardRadius}`);

    let tickCount = 0;

    setInterval(() => {
        try {
            tickCount++;

            // ── AIState (FString UFUNCTION) ──────────────────────────────────
            let aiState = "Unknown";
            try {
                aiState = String(self.GetBehaviacProperty("AIState"));
            } catch(e) {
                aiState = "Error:" + e;
            }

            // ── Position (FVector UPROPERTY via GetActorLocation UFUNCTION) ──
            let posStr = "?";
            try {
                const pos = self.GetActorLocation();
                // FVector.X/Y/Z are wrapped — Number() is the safest coercion
                const px = Math.round(Number(pos.X));
                const py = Math.round(Number(pos.Y));
                const pz = Math.round(Number(pos.Z));
                posStr = `(${px}, ${py}, ${pz})`;
            } catch(e) {
                posStr = "Error:" + e;
            }

            // ── Velocity ─────────────────────────────────────────────────────
            let speed = 0;
            try {
                const vel = self.GetVelocity();
                const vx = Number(vel.X);
                const vy = Number(vel.Y);
                speed = Math.round(Math.sqrt(vx * vx + vy * vy));
            } catch(e) {}

            // ── Target ───────────────────────────────────────────────────────
            let targetStr = "none";
            try {
                const target = self.TargetPlayer;
                if (target) targetStr = String(target.GetName());
            } catch(e) {}

            console.log(
                `[npc_logic][${name}] tick#${tickCount} | ` +
                `State: ${aiState} | ` +
                `Pos: ${posStr} | ` +
                `Speed: ${speed} | ` +
                `Target: ${targetStr}`
            );

            // ── TS override ──────────────────────────────────────────────────
            if (aiState === "Combat" && tickCount % 5 === 0) {
                console.log(`[npc_logic][${name}] 🧠 TS override: StopMovement cooldown`);
                self.StopMovement();
            }

        } catch (e) {
            console.error(`[npc_logic][${name}] ❌ Tick error: ${e}`);
        }
    }, 3000);

    console.log(`[npc_logic] ⏱️  Status logger started (every 3s) for ${name}`);
}
