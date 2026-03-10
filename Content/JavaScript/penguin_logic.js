"use strict";
var _a;
Object.defineProperty(exports, "__esModule", { value: true });
const UE = require("ue");
const Success = 1;
const Failure = 2;
const Running = 3;
const clampAxis = (degrees) => {
    let value = degrees % 360;
    if (value < 0) {
        value += 360;
    }
    return value;
};
const deltaAngleDegrees = (from, to) => {
    let delta = (to - from) % 360;
    if (delta > 180) {
        delta -= 360;
    }
    else if (delta < -180) {
        delta += 360;
    }
    return delta;
};
const createRotationController = (actor) => {
    const getRotation = () => actor.K2_GetActorRotation();
    const setYaw = (yaw) => {
        const current = getRotation();
        const next = new UE.Rotator(current.Pitch, clampAxis(yaw), current.Roll);
        actor.K2_SetActorRotation(next, false);
    };
    const addYaw = (deltaYaw) => {
        setYaw(getRotation().Yaw + deltaYaw);
    };
    return {
        getYaw: () => getRotation().Yaw,
        setYaw,
        addYaw,
    };
};
const createMoodHandler = (actor, actorName) => {
    return () => {
        const roll = Math.random();
        actor.SetMoodRoll(roll);
        const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
        console.log(`[penguin_logic] ${actorName} mood → ${roll.toFixed(2)} ${mood}`);
        return Success;
    };
};
const createWanderHandlers = (actor, actorName) => {
    const spawnX = actor.GetLocationX();
    const spawnY = actor.GetLocationY();
    return {
        "PickWanderTarget": () => {
            const radius = actor.WanderRadius;
            const angle = Math.random() * 2 * Math.PI;
            const dist = radius * (0.3 + Math.random() * 0.7);
            const tx = spawnX + Math.cos(angle) * dist;
            const ty = spawnY + Math.sin(angle) * dist;
            actor.SetNavTarget(tx, ty);
            console.log(`[penguin_logic] ${actorName} → target (${tx.toFixed(0)}, ${ty.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },
        "MoveToWanderTarget": () => {
            const acceptance = actor.WanderAcceptanceRadius;
            const result = actor.NavMoveToTarget(acceptance);
            if (result === 0)
                return Running;
            if (result === 1)
                return Success;
            return Failure;
        },
        "StopMovement": () => {
            actor.NavStop();
            return Success;
        },
    };
};
const createLookAroundHandler = (rotation, actorName) => {
    let lookingAround = false;
    let lookStartTime = 0;
    let lookDurationMs = 0;
    let lookStartYaw = 0;
    let lookTargetYaw = 0;
    return () => {
        const now = Date.now();
        if (!lookingAround) {
            lookDurationMs = 600 + Math.random() * 800;
            lookStartTime = now;
            lookStartYaw = rotation.getYaw();
            lookTargetYaw = clampAxis(lookStartYaw + 30 + Math.random() * 300);
            lookingAround = true;
            console.log(`[penguin_logic] ${actorName} 👀 LookAround ${lookStartYaw.toFixed(1)}° → ${lookTargetYaw.toFixed(1)}° (${lookDurationMs.toFixed(0)}ms)`);
            return Running;
        }
        const alpha = Math.min((now - lookStartTime) / lookDurationMs, 1);
        const delta = deltaAngleDegrees(lookStartYaw, lookTargetYaw);
        rotation.setYaw(lookStartYaw + delta * alpha);
        if (alpha < 1) {
            return Running;
        }
        lookingAround = false;
        return Success;
    };
};
const createSpeedHandlers = (actor) => {
    const setScaledSpeed = (multiplier) => {
        actor.SetMaxSpeed(actor.WanderSpeed * multiplier);
        return Success;
    };
    return {
        "SetSleepySpeed": () => setScaledSpeed(0.6),
        "SetWanderSpeed": () => setScaledSpeed(1.0),
        "SetExcitedSpeed": () => setScaledSpeed(2.5),
    };
};
const createGoofyActionHandlers = (actor, actorName, rotation) => {
    return {
        "MaybeSpin": () => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${actorName} 🌀 MaybeSpin!`);
                rotation.addYaw(90 + Math.random() * 180);
            }
            return Success;
        },
        "SpinAround": () => {
            console.log(`[penguin_logic] ${actorName} 🔄 SpinAround!`);
            rotation.addYaw(180);
            return Success;
        },
        "ExcitedJump": () => {
            console.log(`[penguin_logic] ${actorName} 🐧💨 ExcitedJump!`);
            actor.LaunchCharacter(new UE.Vector(0, 0, 420), false, true);
            return Success;
        },
    };
};
const createHandlers = (actor, actorName) => {
    const rotation = createRotationController(actor);
    return Object.assign(Object.assign(Object.assign(Object.assign({ "RollMood": createMoodHandler(actor, actorName) }, createWanderHandlers(actor, actorName)), { "LookAround": createLookAroundHandler(rotation, actorName) }), createSpeedHandlers(actor)), createGoofyActionHandlers(actor, actorName, rotation));
};
const bindBehaviacHandlers = (behaviacAgent, handlers, actorName) => {
    behaviacAgent.OnMethodNameCalled.Add((methodName) => {
        const handler = handlers[String(methodName)];
        if (!handler) {
            return;
        }
        try {
            behaviacAgent.SetTSMethodResult(methodName, handler());
        }
        catch (e) {
            console.error(`[penguin_logic] ${actorName} ❌ ${methodName}: ${e}`);
            behaviacAgent.SetTSMethodResult(methodName, Failure);
        }
    });
};
const startHeartbeat = (actor, actorName) => {
    setInterval(() => {
        const px = actor.GetLocationX().toFixed(0);
        const py = actor.GetLocationY().toFixed(0);
        const spd = actor.GetSpeedXY().toFixed(0);
        const mood = actor.GetMoodRoll().toFixed(2);
        console.log(`[penguin_logic] ${actorName} 🐧 pos:(${px},${py}) spd:${spd} mood:${mood}`);
    }, 5000);
};
const self = puerts.argv.getByName("self");
const agent = (_a = puerts.argv.getByName("agent")) !== null && _a !== void 0 ? _a : self === null || self === void 0 ? void 0 : self.BehaviacAgent;
if (!self || !agent) {
    console.error(`[penguin_logic] ERROR: missing binding target — self:${!!self} agent:${!!agent}`);
}
else {
    const name = String(self.GetName());
    console.log(`[penguin_logic] ✅ Loaded for: ${name}`);
    const handlers = createHandlers(self, name);
    bindBehaviacHandlers(agent, handlers, name);
    console.log(`[penguin_logic] ✅ Behaviac methods: [${Object.keys(handlers).join(", ")}]`);
    startHeartbeat(self, name);
}
