// npc_logic.ts — The Idiot Brain™
// This NPC is technically functional but deeply unhinged.
const npcSelf = puerts.argv.getByName("self");
const btBridge = puerts.argv.getByName("btBridge");
const ai = puerts.argv.getByName("ai");
if (!npcSelf || !btBridge || !ai) {
    console.error(`[npc_logic] ERROR: missing argv — self:${!!npcSelf} btBridge:${!!btBridge} ai:${!!ai}`);
}
else {
    const name = String(npcSelf.GetName());
    console.log(`[npc_logic] 🧠 Idiot Brain™ online for: ${name}`);
    // ── BT result constants ──────────────────────────────────────────────────
    const Running = 0;
    const Success = 1;
    const Failure = 2;
    // ── Goofy state ──────────────────────────────────────────────────────────
    let panicMode = false;
    let spinCount = 0;
    let crouchTimer = 0;
    let isCrouching = false;
    let lastJumpTime = 0;
    let sprintBurst = false;
    let tauntCooldown = 0;
    let now = () => Date.now();
    // ── Helpers ───────────────────────────────────────────────────────────────
    function rand(min, max) {
        return min + Math.random() * (max - min);
    }
    function chance(pct) {
        return Math.random() < pct;
    }
    // ── Goofy modifiers — fire-and-forget nonsense layered on top of BT ──────
    // These run independently of the BT on a dumb timer.
    setInterval(() => {
        const dist = ai.GetDistanceToPlayer();
        const state = String(ai.GetAIState());
        // Random panic jump when player is close
        if (dist > 0 && dist < 300 && chance(0.4)) {
            console.log(`[${name}] 😱 PANIC JUMP`);
            ai.LaunchUp(rand(400, 800));
            lastJumpTime = now();
        }
        // Random crouch/uncrouch cycle
        if (chance(0.25)) {
            if (!isCrouching) {
                console.log(`[${name}] 🦆 Crouching randomly`);
                ai.Crouch();
                isCrouching = true;
                crouchTimer = now();
            }
        }
        if (isCrouching && now() - crouchTimer > rand(500, 2000)) {
            ai.UnCrouch();
            isCrouching = false;
        }
        // Spin when idle/patrolling (existential crisis)
        if (state === "Patrol" && chance(0.2)) {
            spinCount = Math.floor(rand(2, 6));
            console.log(`[${name}] 🌀 Spinning ${spinCount} times (for no reason)`);
        }
        // Random sprint burst
        if (state === "Chase" && chance(0.3) && !sprintBurst) {
            console.log(`[${name}] 💨 SPRINT BURST`);
            ai.SetSpeedRaw(rand(900, 1400));
            sprintBurst = true;
            setTimeout(() => {
                ai.SetSpeed(ai.RunSpeed);
                sprintBurst = false;
            }, rand(400, 900));
        }
        // Panic mode: triggered when health is low (or just randomly, because idiot)
        if (!panicMode && chance(0.05)) {
            panicMode = true;
            console.log(`[${name}] 🆘 ENTERING PANIC MODE (randomly)`);
            setTimeout(() => {
                panicMode = false;
                console.log(`[${name}] 😌 Panic over. Resuming idiocy.`);
            }, rand(3000, 6000));
        }
        // Taunt when player is nearby (jump + face them + spin)
        if (dist > 0 && dist < 500 && now() - tauntCooldown > 8000 && chance(0.15)) {
            tauntCooldown = now();
            console.log(`[${name}] 💃 TAUNT: You can't catch me!`);
            ai.FaceTarget();
            ai.LaunchUp(300);
            setTimeout(() => ai.Spin(180), 300);
            setTimeout(() => ai.Spin(180), 600);
        }
    }, 500);
    // Spin executor — applies pending spins every 200ms
    setInterval(() => {
        if (spinCount > 0) {
            ai.Spin(72); // 5 spins = 360°
            spinCount--;
        }
    }, 200);
    // ── BT action handlers ───────────────────────────────────────────────────
    const handlers = {
        "UpdateAIState": () => {
            const distFromPost = ai.GetDistanceFromPost();
            const distToPlayer = ai.GetDistanceToPlayer();
            const canSee = ai.CanSeePlayer();
            const currentState = String(ai.GetAIState());
            let newState = "Patrol";
            if (canSee && distToPlayer <= ai.AttackRange) {
                newState = "Combat";
                ai.SetLastKnownPos();
            }
            else if (canSee && distToPlayer <= ai.DetectionRadius) {
                // In panic mode: occasionally runs AWAY instead of chasing
                if (panicMode && chance(0.5)) {
                    newState = "Flee";
                }
                else {
                    newState = "Chase";
                }
                ai.SetLastKnownPos();
            }
            else if (currentState === "Chase" || currentState === "Combat") {
                newState = (distFromPost > ai.GuardRadius) ? "ReturnToPost" : "Investigate";
            }
            else if (distFromPost > ai.GuardRadius) {
                newState = "ReturnToPost";
            }
            ai.SetAIState(newState);
            return Success;
        },
        "SetWalkSpeed": () => { ai.SetSpeed(ai.WalkSpeed); return Success; },
        "SetRunSpeed": () => {
            // Idiot: sometimes sets the wrong speed
            if (chance(0.1)) {
                console.log(`[${name}] 🤪 SetRunSpeed: set walk speed by mistake`);
                ai.SetSpeed(ai.WalkSpeed * 0.5);
            }
            else {
                ai.SetSpeed(ai.RunSpeed);
            }
            return Success;
        },
        "FindPlayer": () => {
            if (ai.CanSeePlayer()) {
                ai.SetLastKnownPos();
                return Success;
            }
            // Idiot: spin while searching
            if (chance(0.4))
                ai.Spin(rand(30, 90));
            return Failure;
        },
        "Patrol": () => {
            ai.Patrol();
            // Idiot: randomly crouch-walks during patrol
            if (chance(0.05)) {
                console.log(`[${name}] 🦆 Crouch-walking on patrol`);
                ai.Crouch();
                isCrouching = true;
                crouchTimer = now();
            }
            return Running;
        },
        "MoveToTarget": () => {
            const dist = ai.GetDistanceToTarget();
            if (dist < 0)
                return Failure;
            if (dist <= ai.AttackRange)
                return Success;
            ai.MoveToTarget();
            return Running;
        },
        "ChasePlayer": () => {
            const state = String(ai.GetAIState());
            if (state === "Flee") {
                // Run AWAY — dash in opposite direction
                console.log(`[${name}] 🏃 FLEEING like a coward`);
                ai.Spin(180);
                ai.SetSpeedRaw(ai.RunSpeed * 1.5);
                ai.Dash(800);
                return Running;
            }
            if (state !== "Chase") {
                ai.StopMovement();
                return Failure;
            }
            const dist = ai.GetDistanceToTarget();
            if (dist < 0)
                return Failure;
            if (dist <= ai.AttackRange)
                return Success;
            // Panic chase: random jumps while running
            if (panicMode && chance(0.2)) {
                console.log(`[${name}] 😱 Panic-jumping while chasing`);
                ai.LaunchUp(300);
            }
            ai.MoveToTarget();
            return Running;
        },
        "AttackPlayer": () => {
            const state = String(ai.GetAIState());
            if (state !== "Combat")
                return Failure;
            const dist = ai.GetDistanceToTarget();
            if (dist < 0 || dist > ai.CombatRange)
                return Failure;
            // Idiot attack: sometimes jumps instead of attacking
            if (chance(0.25)) {
                console.log(`[${name}] 🤦 AttackPlayer: jumped instead of attacking`);
                ai.LaunchUp(500);
                return Running; // missed, try again
            }
            console.log(`[${name}] ⚔️ HIT! dist=${Math.round(dist)}`);
            // Celebratory spin after landing a hit
            if (chance(0.5)) {
                setTimeout(() => {
                    console.log(`[${name}] 🎉 Victory spin!`);
                    ai.Spin(360);
                }, 200);
            }
            return Success;
        },
        "FaceTarget": () => { ai.FaceTarget(); return Success; },
        "StopMovement": () => { ai.StopMovement(); return Success; },
        "MoveToLastKnownPos": () => {
            const arrived = ai.MoveToLastKnownPos();
            // Idiot: sometimes spins while walking to last known pos
            if (chance(0.1))
                ai.Spin(rand(20, 60));
            return arrived ? Success : Running;
        },
        "LookAround": () => {
            ai.LookAround();
            // Extra: random extra spins when looking around
            if (chance(0.3))
                ai.Spin(rand(45, 135));
            return Success;
        },
        "ClearLastKnownPos": () => {
            ai.ClearLastKnownPos();
            ai.SetAIState("Patrol");
            return Success;
        },
        "ReturnToPost": () => {
            if (ai.GetDistanceFromPost() < 100) {
                ai.SetAIState("Patrol");
                // Return home celebration: little jump
                if (chance(0.6))
                    ai.LaunchUp(200);
                return Success;
            }
            ai.SetSpeed(ai.WalkSpeed);
            ai.MoveToPost();
            return Running;
        },
    };
    // ── Bind to BT dispatch delegate ─────────────────────────────────────────
    btBridge.OnBTAction.Add((actionName) => {
        const handler = handlers[String(actionName)];
        if (handler) {
            try {
                btBridge.SetBTResult(handler());
            }
            catch (e) {
                console.error(`[npc_logic][${name}] ❌ ${actionName}: ${e}`);
                btBridge.SetBTResult(Failure);
            }
        }
        // No handler → sentinel → C++ fallback
    });
    console.log(`[npc_logic] ✅ Handlers: [${Object.keys(handlers).join(", ")}]`);
    // ── Status logger (every 3s) ─────────────────────────────────────────────
    let tick = 0;
    setInterval(() => {
        try {
            tick++;
            const state = String(ai.GetAIState());
            const px = Math.round(ai.GetLocationX());
            const py = Math.round(ai.GetLocationY());
            const speed = Math.round(ai.GetSpeedXY());
            const tStr = ai.TargetActor ? String(ai.TargetActor.GetName()) : "none";
            const mood = panicMode ? "😱PANIC" : isCrouching ? "🦆DUCK" : "😐normal";
            console.log(`[npc_logic][${name}] #${tick} | ${state} | (${px},${py}) | spd:${speed} | tgt:${tStr} | ${mood}`);
        }
        catch (e) { /* swallow */ }
    }, 3000);
}
