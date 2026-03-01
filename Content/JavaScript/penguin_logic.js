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
    // LookAround: JS drives the turn duration with a timer.
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
        // ── Navigation — fall through to C++ for all nav calls ───────────
        // JS doesn't have access to AIController/pathfinding.
        // C++ CPP_PickWanderTarget sets bHasWanderTarget; CPP_MoveToWanderTarget
        // issues MoveToLocation and returns Running until arrived.
        "PickWanderTarget": () => {
            return FALLTHROUGH; // C++ handles target selection + bHasWanderTarget
        },
        "MoveToWanderTarget": () => {
            return FALLTHROUGH; // C++ handles AIController.MoveToLocation
        },
        "StopMovement": () => {
            return FALLTHROUGH; // C++ handles AIController.StopMovement
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
