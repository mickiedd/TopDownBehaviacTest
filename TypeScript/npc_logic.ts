// npc_logic.ts

declare const puerts: any;
declare function setInterval(fn: () => void, ms: number): any;

const self: any = puerts.argv.getByName("self");

if (!self) {
    console.error("[npc_logic] ERROR: 'self' not found in argv");
} else {
    const name: string = String(self.GetName());
    console.log(`[npc_logic] ✅ Script loaded for NPC: ${name}`);
    console.log(`[npc_logic] 📋 Config — DetectionRadius: ${self.DetectionRadius}, WalkSpeed: ${self.WalkSpeed}, RunSpeed: ${self.RunSpeed}, GuardRadius: ${self.GuardRadius}`);

    let tickCount = 0;

    setInterval(() => {
        try {
            tickCount++;

            const aiState: string = String(self.GetBehaviacProperty("AIState"));

            // Primitive helpers avoid FVector struct boundary issues
            const px = Math.round(self.GetLocationX());
            const py = Math.round(self.GetLocationY());
            const pz = Math.round(self.GetLocationZ());
            const speed = Math.round(self.GetSpeedXY());

            const target = self.TargetPlayer;
            const targetStr = target ? String(target.GetName()) : "none";

            console.log(
                `[npc_logic][${name}] tick#${tickCount} | ` +
                `State: ${aiState} | ` +
                `Pos: (${px}, ${py}, ${pz}) | ` +
                `Speed: ${speed} | ` +
                `Target: ${targetStr}`
            );

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
