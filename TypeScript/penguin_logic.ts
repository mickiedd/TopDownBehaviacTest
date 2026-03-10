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

declare function require(name: string): any;

const UE: any = require("ue");
const Success = 1;
const Failure = 2;
const Running = 3;

type Handler = () => number;
type HandlerMap = Record<string, Handler>;

const clampAxis = (degrees: number): number => {
    let value = degrees % 360;
    if (value < 0) {
        value += 360;
    }
    return value;
};

const deltaAngleDegrees = (from: number, to: number): number => {
    let delta = (to - from) % 360;
    if (delta > 180) {
        delta -= 360;
    } else if (delta < -180) {
        delta += 360;
    }
    return delta;
};

const createRotationController = (actor: any) => {
    const getRotation = (): any => actor.K2_GetActorRotation();

    const setYaw = (yaw: number): void => {
        const current = getRotation();
        const next = new UE.Rotator(current.Pitch, clampAxis(yaw), current.Roll);
        actor.K2_SetActorRotation(next, false);
    };

    const addYaw = (deltaYaw: number): void => {
        setYaw(getRotation().Yaw + deltaYaw);
    };

    return {
        getYaw: (): number => getRotation().Yaw,
        setYaw,
        addYaw,
    };
};

const createMoodHandler = (actor: any, actorName: string): Handler => {
    return (): number => {
        const roll = Math.random();
        actor.SetMoodRoll(roll);
        const mood = roll < 0.35 ? "😴 Sleepy" : roll < 0.70 ? "🐧 Curious" : "🎉 Excited";
        console.log(`[penguin_logic] ${actorName} mood → ${roll.toFixed(2)} ${mood}`);
        return Success;
    };
};

const createWanderHandlers = (actor: any, actorName: string): HandlerMap => {
    const spawnX: number = actor.GetLocationX() as number;
    const spawnY: number = actor.GetLocationY() as number;

    return {
        "PickWanderTarget": (): number => {
            const radius = actor.WanderRadius as number;
            const angle = Math.random() * 2 * Math.PI;
            const dist = radius * (0.3 + Math.random() * 0.7);
            const tx = spawnX + Math.cos(angle) * dist;
            const ty = spawnY + Math.sin(angle) * dist;
            actor.SetNavTarget(tx, ty);
            console.log(`[penguin_logic] ${actorName} → target (${tx.toFixed(0)}, ${ty.toFixed(0)}) dist=${dist.toFixed(0)}`);
            return Success;
        },

        "MoveToWanderTarget": (): number => {
            const acceptance = actor.WanderAcceptanceRadius as number;
            const result = actor.NavMoveToTarget(acceptance) as number;
            if (result === 0) return Running;
            if (result === 1) return Success;
            return Failure;
        },

        "StopMovement": (): number => {
            actor.NavStop();
            return Success;
        },
    };
};

const createLookAroundHandler = (rotation: ReturnType<typeof createRotationController>, actorName: string): Handler => {
    let lookingAround = false;
    let lookStartTime = 0;
    let lookDurationMs = 0;
    let lookStartYaw = 0;
    let lookTargetYaw = 0;

    return (): number => {
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

const createSpeedHandlers = (actor: any): HandlerMap => {
    const setScaledSpeed = (multiplier: number): number => {
        actor.SetMaxSpeed((actor.WanderSpeed as number) * multiplier);
        return Success;
    };

    return {
        "SetSleepySpeed": (): number => setScaledSpeed(0.6),
        "SetWanderSpeed": (): number => setScaledSpeed(1.0),
        "SetExcitedSpeed": (): number => setScaledSpeed(2.5),
    };
};

const createGoofyActionHandlers = (actor: any, actorName: string, rotation: ReturnType<typeof createRotationController>): HandlerMap => {
    return {
        "MaybeSpin": (): number => {
            if (Math.random() < 0.4) {
                console.log(`[penguin_logic] ${actorName} 🌀 MaybeSpin!`);
                rotation.addYaw(90 + Math.random() * 180);
            }
            return Success;
        },

        "SpinAround": (): number => {
            console.log(`[penguin_logic] ${actorName} 🔄 SpinAround!`);
            rotation.addYaw(180);
            return Success;
        },

        "ExcitedJump": (): number => {
            console.log(`[penguin_logic] ${actorName} 🐧💨 ExcitedJump!`);
            actor.LaunchCharacter(new UE.Vector(0, 0, 420), false, true);
            return Success;
        },
    };
};

const createHandlers = (actor: any, actorName: string): HandlerMap => {
    const rotation = createRotationController(actor);

    return {
        "RollMood": createMoodHandler(actor, actorName),
        ...createWanderHandlers(actor, actorName),
        "LookAround": createLookAroundHandler(rotation, actorName),
        ...createSpeedHandlers(actor),
        ...createGoofyActionHandlers(actor, actorName, rotation),
    };
};

const bindBehaviacHandlers = (behaviacAgent: any, handlers: HandlerMap, actorName: string): void => {
    behaviacAgent.OnMethodNameCalled.Add((methodName: string) => {
        const handler = handlers[String(methodName)];
        if (!handler) {
            return;
        }

        try {
            behaviacAgent.SetTSMethodResult(methodName, handler());
        } catch (e) {
            console.error(`[penguin_logic] ${actorName} ❌ ${methodName}: ${e}`);
            behaviacAgent.SetTSMethodResult(methodName, Failure);
        }
    });
};

const startHeartbeat = (actor: any, actorName: string): void => {
    setInterval(() => {
        const px = (actor.GetLocationX() as number).toFixed(0);
        const py = (actor.GetLocationY() as number).toFixed(0);
        const spd = (actor.GetSpeedXY() as number).toFixed(0);
        const mood = (actor.GetMoodRoll() as number).toFixed(2);
        console.log(`[penguin_logic] ${actorName} 🐧 pos:(${px},${py}) spd:${spd} mood:${mood}`);
    }, 5000);
};

const self: any     = puerts.argv.getByName("self");
const agent: any    = puerts.argv.getByName("agent") ?? self?.BehaviacAgent;

if (!self || !agent) {
    console.error(`[penguin_logic] ERROR: missing binding target — self:${!!self} agent:${!!agent}`);
} else {
    const name: string = String(self.GetName());
    console.log(`[penguin_logic] ✅ Loaded for: ${name}`);
    const handlers = createHandlers(self, name);

    bindBehaviacHandlers(agent, handlers, name);

    console.log(`[penguin_logic] ✅ Behaviac methods: [${Object.keys(handlers).join(", ")}]`);
    startHeartbeat(self, name);
}
