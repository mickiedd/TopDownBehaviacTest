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
    const FALLTHROUGH = -2147483648; // INT32_MIN → DispatchOrRun uses C++ fallback
    // ── State ──────────────────────────────────────────────────────────────
    // Spawn position cached once (for wander radius math)
    const spawnX = self.GetLocationX();
    const spawnY = self.GetLocationY();
    // LookAround: JS timer-based (avoids C++ bLookAroundComplete state)
    let lookingAround = false;
    let lookEndTime = 0;
    // ── BT handlers ────────────────────────────────────────────────────────
    const handlers = {
        // ── Mood ──────────────────────────────────────────────────────────
        "RollMood": () => {
            const roll = Math.random();
            self.SetMoodRoll(roll);
            const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
            console.log(`[penguin_logic] ${name} mood → ${roll.toFixed(2)} ${mood}`);
            return Success;
        },
        // ── Navigation (full ownership via base class nav API) ─────────────
        "PickWanderTarget": () => {
            const radius = self.WanderRadius;
            const angle = Math.random() * 2 * Math.PI;
            const dist = radius * (0.3 + Math.random() * 0.7);
            const tx = spawnX + Math.cos(angle) * dist;
            const ty = spawnY + Math.sin(angle) * dist;
            self.SetNavTarget(tx, ty);
            console.log(`[penguin_logic] ${name} → target (${tx.toFixed(0)}, ${ty.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },
        "MoveToWanderTarget": () => {
            const acceptance = self.WanderAcceptanceRadius;
            const result = self.NavMoveToTarget(acceptance);
            // 0=Running, 1=Success, 2=Failure
            return result;
        },
        "StopMovement": () => {
            self.NavStop();
            return Success;
        },
        // ── LookAround — JS timer (600–1400ms natural turn feel) ──────────
        "LookAround": () => {
            const now = Date.now();
            if (!lookingAround) {
                const duration = 600 + Math.random() * 800;
                lookEndTime = now + duration;
                lookingAround = true;
                console.log(`[penguin_logic] ${name} 👀 LookAround (${duration.toFixed(0)}ms)`);
                return Running;
            }
            if (now < lookEndTime)
                return Running;
            lookingAround = false;
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
        // ── Goofy actions — log in JS, physics in C++ via FALLTHROUGH ─────
        "MaybeSpin": () => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${name} 🌀 MaybeSpin!`);
                return FALLTHROUGH; // C++ does the rotation snap
            }
            return Success; // skipped this time
        },
        "SpinAround": () => {
            console.log(`[penguin_logic] ${name} 🔄 SpinAround!`);
            return FALLTHROUGH; // C++ does the rotation
        },
        "ExcitedJump": () => {
            console.log(`[penguin_logic] ${name} 🐧💨 ExcitedJump!`);
            return FALLTHROUGH; // C++ does LaunchCharacter
        },
    };
    // ── BT dispatch binding ────────────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[actionName];
        if (handler) {
            btBridge.SetBTResult(handler());
        }
        else {
            btBridge.SetBTResult(FALLTHROUGH); // unknown — let C++ handle
        }
    });
    // ── Heartbeat log (every 5s) ───────────────────────────────────────────
    setInterval(() => {
        const px = self.GetLocationX().toFixed(0);
        const py = self.GetLocationY().toFixed(0);
        const spd = self.GetSpeedXY().toFixed(0);
        const mood = self.GetMoodRoll().toFixed(2);
        console.log(`[penguin_logic] ${name} 🐧 pos:(${px},${py}) spd:${spd} mood:${mood}`);
    }, 5000);
}
