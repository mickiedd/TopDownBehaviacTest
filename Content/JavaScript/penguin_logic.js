"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const self = puerts.argv.getByName("self");
const btBridge = puerts.argv.getByName("btBridge");
if (!self || !btBridge) {
    console.error(`[penguin_logic] ERROR: missing argv — self:${!!self} btBridge:${!!btBridge}`);
}
else {
    const name = String(self.GetName());
    console.log(`[penguin_logic] ✅ Loaded for: ${name}`);
    const Running = 0;
    const Success = 1;
    const Failure = 2;
    // ── State ──────────────────────────────────────────────────────────────
    // Wander target: penguin picks one each cycle and navigates to it.
    // SpawnX/Y are cached for radius math.
    const spawnX = self.GetLocationX();
    const spawnY = self.GetLocationY();
    let targetX = spawnX;
    let targetY = spawnY;
    let hasTarget = false;
    // LookAround: JS drives timing via a settled flag (same idea as C++ bLookAroundComplete)
    let lookStartTime = 0;
    let lookDurationMs = 0;
    let lookingAround = false;
    // ── Helpers ────────────────────────────────────────────────────────────
    function dist2D(ax, ay, bx, by) {
        const dx = ax - bx, dy = ay - by;
        return Math.sqrt(dx * dx + dy * dy);
    }
    // Fall-through sentinel: DispatchOrRun sees INT32_MIN and runs C++ fallback instead
    const FALLTHROUGH = -2147483648;
    const handlers = {
        // ── Mood ──────────────────────────────────────────────────────────
        "RollMood": () => {
            const roll = Math.random();
            self.SetMoodRoll(roll);
            const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
            console.log(`[penguin_logic] ${name} mood → ${roll.toFixed(2)} ${mood}`);
            return Success;
        },
        // ── Navigation ────────────────────────────────────────────────────
        "PickWanderTarget": () => {
            const radius = self.WanderRadius;
            const angle = Math.random() * 2 * Math.PI;
            const dist = radius * (0.3 + Math.random() * 0.7);
            targetX = spawnX + Math.cos(angle) * dist;
            targetY = spawnY + Math.sin(angle) * dist;
            hasTarget = true;
            console.log(`[penguin_logic] ${name} → target (${targetX.toFixed(0)}, ${targetY.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },
        "MoveToWanderTarget": () => {
            if (!hasTarget)
                return Failure;
            const px = self.GetLocationX();
            const py = self.GetLocationY();
            const acceptance = self.WanderAcceptanceRadius;
            if (dist2D(px, py, targetX, targetY) <= acceptance)
                return Success;
            // Fall through to C++ for actual AIController.MoveToLocation
            return FALLTHROUGH; // INT32_MIN → DispatchOrRun falls back to C++
        },
        "StopMovement": () => {
            hasTarget = false;
            // Fall through to C++ for AIController.StopMovement
            return FALLTHROUGH; // INT32_MIN → fall through to C++
        },
        // ── LookAround: JS timer-based (no bLookAroundComplete state in C++) ──
        "LookAround": () => {
            const now = Date.now();
            if (!lookingAround) {
                // Start: pick a random turn duration (600–1400ms feels natural)
                lookDurationMs = 600 + Math.random() * 800;
                lookStartTime = now;
                lookingAround = true;
                console.log(`[penguin_logic] ${name} LookAround start (${lookDurationMs.toFixed(0)}ms)`);
                return Running;
            }
            if (now - lookStartTime < lookDurationMs)
                return Running;
            // Done
            lookingAround = false;
            console.log(`[penguin_logic] ${name} LookAround done`);
            return Success;
        },
        // ── Speed setters ─────────────────────────────────────────────────
        "SetSleepySpeed": () => {
            self.SetMaxSpeed(self.WanderSpeed * 0.6);
            return Success;
        },
        "SetWanderSpeed": () => {
            self.SetMaxSpeed(self.WanderSpeed);
            return Success;
        },
        "SetExcitedSpeed": () => {
            self.SetMaxSpeed(self.WanderSpeed * 2.5);
            return Success;
        },
        // ── Goofy actions ─────────────────────────────────────────────────
        "MaybeSpin": () => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${name} 🌀 MaybeSpin!`);
                return FALLTHROUGH; // fall through → C++ does the rotation snap
            }
            return Success; // skipped this time
        },
        "SpinAround": () => {
            console.log(`[penguin_logic] ${name} 🔄 SpinAround!`);
            return FALLTHROUGH; // fall through → C++ does the rotation
        },
        "ExcitedJump": () => {
            console.log(`[penguin_logic] ${name} 🐧💨 ExcitedJump!`);
            return FALLTHROUGH; // fall through → C++ does LaunchCharacter
        },
    };
    // ── BT dispatch binding ────────────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[actionName];
        if (handler) {
            btBridge.SetBTResult(handler());
        }
        else {
            // Unknown action — let C++ fallback handle it
            btBridge.SetBTResult(FALLTHROUGH);
        }
    });
    // ── Heartbeat log (every 5s) ───────────────────────────────────────────
    setInterval(() => {
        const px = self.GetLocationX();
        const py = self.GetLocationY();
        const spd = self.GetSpeedXY();
        const mood = self.GetMoodRoll().toFixed(2);
        console.log(`[penguin_logic] ${name} 🐧 pos:(${px.toFixed(0)},${py.toFixed(0)}) spd:${spd.toFixed(0)} mood:${mood}`);
    }, 5000);
}
