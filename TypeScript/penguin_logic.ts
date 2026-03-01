// penguin_logic.ts — TypeScript action implementations for PenguinWanderTree.
//
// Rule: TypeScript ONLY implements what a BT leaf node asks for.
//       All decisions, mood branching, timing, and sequencing live in PenguinWanderTree.xml.
//
// Argv injected by UPuertsNPCComponent (ScriptModule = "penguin_logic"):
//   self     → ABehaviacPenguin (actor)
//   btBridge → UPuertsNPCComponent (BT dispatch + SetBTResult)
export {};

const self: any     = puerts.argv.getByName("self");
const btBridge: any = puerts.argv.getByName("btBridge");

if (!self || !btBridge) {
    console.error(`[penguin_logic] ERROR: missing argv — self:${!!self} btBridge:${!!btBridge}`);
} else {
    const name: string = String(self.GetName());
    console.log(`[penguin_logic] ✅ Loaded for: ${name}`);

    const Running = 0;
    const Success = 1;
    const Failure = 2;

    // ── State ──────────────────────────────────────────────────────────────
    // Wander target: penguin picks one each cycle and navigates to it.
    // SpawnX/Y are cached for radius math.
    const spawnX: number = self.GetLocationX();
    const spawnY: number = self.GetLocationY();
    let targetX = spawnX;
    let targetY = spawnY;
    let hasTarget = false;

    // LookAround: JS drives timing via a settled flag (same idea as C++ bLookAroundComplete)
    let lookStartTime    = 0;
    let lookDurationMs   = 0;
    let lookingAround    = false;

    // ── Helpers ────────────────────────────────────────────────────────────
    function dist2D(ax: number, ay: number, bx: number, by: number): number {
        const dx = ax - bx, dy = ay - by;
        return Math.sqrt(dx * dx + dy * dy);
    }

    // Fall-through sentinel: DispatchOrRun sees INT32_MIN and runs C++ fallback instead
    const FALLTHROUGH = -2147483648;
    const handlers: Record<string, () => number> = {

        // ── Mood ──────────────────────────────────────────────────────────
        "RollMood": (): number => {
            const roll = Math.random();
            self.SetMoodRoll(roll);
            const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
            console.log(`[penguin_logic] ${name} mood → ${roll.toFixed(2)} ${mood}`);
            return Success;
        },

        // ── Navigation ────────────────────────────────────────────────────
        "PickWanderTarget": (): number => {
            const radius   = self.WanderRadius as number;
            const angle    = Math.random() * 2 * Math.PI;
            const dist     = radius * (0.3 + Math.random() * 0.7);
            targetX = spawnX + Math.cos(angle) * dist;
            targetY = spawnY + Math.sin(angle) * dist;
            hasTarget = true;
            console.log(`[penguin_logic] ${name} → target (${targetX.toFixed(0)}, ${targetY.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },

        "MoveToWanderTarget": (): number => {
            if (!hasTarget) return Failure;
            const px = self.GetLocationX() as number;
            const py = self.GetLocationY() as number;
            const acceptance = self.WanderAcceptanceRadius as number;

            if (dist2D(px, py, targetX, targetY) <= acceptance) return Success;

            // Fall through to C++ for actual AIController.MoveToLocation
            return FALLTHROUGH; // INT32_MIN → DispatchOrRun falls back to C++
        },

        "StopMovement": (): number => {
            hasTarget = false;
            // Fall through to C++ for AIController.StopMovement
            return FALLTHROUGH; // INT32_MIN → fall through to C++
        },

        // ── LookAround: JS timer-based (no bLookAroundComplete state in C++) ──
        "LookAround": (): number => {
            const now = Date.now();
            if (!lookingAround) {
                // Start: pick a random turn duration (600–1400ms feels natural)
                lookDurationMs = 600 + Math.random() * 800;
                lookStartTime  = now;
                lookingAround  = true;
                console.log(`[penguin_logic] ${name} LookAround start (${lookDurationMs.toFixed(0)}ms)`);
                return Running;
            }
            if (now - lookStartTime < lookDurationMs) return Running;
            // Done
            lookingAround = false;
            console.log(`[penguin_logic] ${name} LookAround done`);
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

        // ── Goofy actions ─────────────────────────────────────────────────
        "MaybeSpin": (): number => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${name} 🌀 MaybeSpin!`);
                return FALLTHROUGH; // fall through → C++ does the rotation snap
            }
            return Success; // skipped this time
        },

        "SpinAround": (): number => {
            console.log(`[penguin_logic] ${name} 🔄 SpinAround!`);
            return FALLTHROUGH; // fall through → C++ does the rotation
        },

        "ExcitedJump": (): number => {
            console.log(`[penguin_logic] ${name} 🐧💨 ExcitedJump!`);
            return FALLTHROUGH; // fall through → C++ does LaunchCharacter
        },
    };

    // ── BT dispatch binding ────────────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName: string) => {
        const handler = handlers[actionName];
        if (handler) {
            btBridge.SetBTResult(handler());
        } else {
            // Unknown action — let C++ fallback handle it
            btBridge.SetBTResult(FALLTHROUGH);
        }
    });

    // ── Heartbeat log (every 5s) ───────────────────────────────────────────
    setInterval(() => {
        const px = self.GetLocationX() as number;
        const py = self.GetLocationY() as number;
        const spd = self.GetSpeedXY() as number;
        const mood = (self.GetMoodRoll() as number).toFixed(2);
        console.log(`[penguin_logic] ${name} 🐧 pos:(${px.toFixed(0)},${py.toFixed(0)}) spd:${spd.toFixed(0)} mood:${mood}`);
    }, 5000);
}
