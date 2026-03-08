// penguin_logic.ts — TypeScript behavior-method bindings for PenguinWanderTree.
//
// Rule: TypeScript ONLY implements what a BT leaf node asks for.
//       All decisions, mood branching, timing, and sequencing live in PenguinWanderTree.xml.
//
// Nav API (from ABehaviacAnimalBase — all scalar, safe across Puerts boundary):
//   self.SetNavTarget(x, y)          — store destination
//   self.NavMoveToTarget(acceptance) — 0=Running, 1=Success, 2=Failure
//   self.NavStop()                   — stop + clear target
//   self.GetNavTargetX/Y()           — read stored target
//   self.GetLocationX/Y()            — current position
//   self.GetSpeedXY()                — 2D speed
//   self.SetMaxSpeed(s)              — set walk speed
//
// Argv: self → ABehaviacPenguin, agent → UBehaviacAgentComponent
export {};

const self: any     = puerts.argv.getByName("self");
const agent: any    = puerts.argv.getByName("agent") ?? self?.BehaviacAgent;

if (!self || !agent) {
    console.error(`[penguin_logic] ERROR: missing binding target — self:${!!self} agent:${!!agent}`);
} else {
    const name: string = String(self.GetName());
    console.log(`[penguin_logic] ✅ Loaded for: ${name}`);

    const Success = 1;
    const Failure = 2;
    const Running = 3;

    // ── State ──────────────────────────────────────────────────────────────
    // Spawn position cached once (for wander radius math)
    const spawnX: number = self.GetLocationX() as number;
    const spawnY: number = self.GetLocationY() as number;

    // LookAround: JS timer-based (avoids C++ bLookAroundComplete state)
    let lookingAround  = false;
    let lookEndTime    = 0;

    // ── BT handlers ────────────────────────────────────────────────────────
    const handlers: Record<string, () => number> = {

        // ── Mood ──────────────────────────────────────────────────────────
        "RollMood": (): number => {
            const roll = Math.random();
            self.SetMoodRoll(roll);
            const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
            console.log(`[penguin_logic] ${name} mood → ${roll.toFixed(2)} ${mood}`);
            return Success;
        },

        // ── Navigation (full ownership via base class nav API) ─────────────
        "PickWanderTarget": (): number => {
            const radius = self.WanderRadius as number;
            const angle  = Math.random() * 2 * Math.PI;
            const dist   = radius * (0.3 + Math.random() * 0.7);
            const tx = spawnX + Math.cos(angle) * dist;
            const ty = spawnY + Math.sin(angle) * dist;
            self.SetNavTarget(tx, ty);
            console.log(`[penguin_logic] ${name} → target (${tx.toFixed(0)}, ${ty.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },

        "MoveToWanderTarget": (): number => {
            const acceptance = self.WanderAcceptanceRadius as number;
            const result = self.NavMoveToTarget(acceptance) as number;
            if (result === 0) return Running;
            if (result === 1) return Success;
            return Failure;
        },

        "StopMovement": (): number => {
            self.NavStop();
            return Success;
        },

        // ── LookAround — JS timer (600–1400ms natural turn feel) ──────────
        "LookAround": (): number => {
            const now = Date.now();
            if (!lookingAround) {
                const duration = 600 + Math.random() * 800;
                lookEndTime   = now + duration;
                lookingAround = true;
                console.log(`[penguin_logic] ${name} 👀 LookAround (${duration.toFixed(0)}ms)`);
                return Running;
            }
            if (now < lookEndTime) return Running;
            lookingAround = false;
            return Success;
        },

        // ── Speed setters ─────────────────────────────────────────────────
        "SetSleepySpeed": (): number => {
            self.SetMaxSpeed((self.WanderSpeed as number) * 0.6);
            return Success;
        },

        "SetWanderSpeed": (): number => {
            self.SetMaxSpeed(self.WanderSpeed as number);
            return Success;
        },

        "SetExcitedSpeed": (): number => {
            self.SetMaxSpeed((self.WanderSpeed as number) * 2.5);
            return Success;
        },

        // ── Goofy actions — log in JS, perform native action in C++ ───────
        "MaybeSpin": (): number => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${name} 🌀 MaybeSpin!`);
                return self.MaybeSpin() as number;
            }
            return Success; // skipped this time
        },

        "SpinAround": (): number => {
            console.log(`[penguin_logic] ${name} 🔄 SpinAround!`);
            return self.SpinAround() as number;
        },

        "ExcitedJump": (): number => {
            console.log(`[penguin_logic] ${name} 🐧💨 ExcitedJump!`);
            return self.ExcitedJump() as number;
        },
    };

    // ── Behaviac method binding ────────────────────────────────────────────
    agent.OnMethodNameCalled.Add((methodName: string) => {
        const handler = handlers[String(methodName)];
        if (!handler) {
            return;
        }

        try {
            agent.SetTSMethodResult(methodName, handler());
        } catch (e) {
            console.error(`[penguin_logic] ${name} ❌ ${methodName}: ${e}`);
            agent.SetTSMethodResult(methodName, Failure);
        }
    });

    console.log(`[penguin_logic] ✅ Behaviac methods: [${Object.keys(handlers).join(", ")}]`);

    // ── Heartbeat log (every 5s) ───────────────────────────────────────────
    setInterval(() => {
        const px   = (self.GetLocationX() as number).toFixed(0);
        const py   = (self.GetLocationY() as number).toFixed(0);
        const spd  = (self.GetSpeedXY()   as number).toFixed(0);
        const mood = (self.GetMoodRoll()  as number).toFixed(2);
        console.log(`[penguin_logic] ${name} 🐧 pos:(${px},${py}) spd:${spd} mood:${mood}`);
    }, 5000);
}
