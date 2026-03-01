"use strict";
// npc_diag.js — one-shot diagnostic: tests each NPC method individually
// Set ScriptModule = "npc_diag" on the PuertsNPCComponent temporarily

const self = puerts.argv.getByName("self");
if (!self) { console.error("[diag] No self!"); }
else {
    console.log("[diag] --- NPC Diagnostics ---");

    const tests = [
        ["GetName",             () => self.GetName()],
        ["DetectionRadius",     () => self.DetectionRadius],
        ["WalkSpeed",           () => self.WalkSpeed],
        ["TargetPlayer",        () => self.TargetPlayer],
        ["GetActorLocation",    () => { const p = self.GetActorLocation(); return `X=${p.X} Y=${p.Y} Z=${p.Z}`; }],
        ["GetVelocity",         () => { const v = self.GetVelocity(); return `X=${v.X} Y=${v.Y}`; }],
        ["GetBehaviacProperty", () => self.GetBehaviacProperty("AIState")],
        ["Number(pos.X)",       () => { const p = self.GetActorLocation(); return Number(p.X); }],
        ["Math on pos.X",       () => { const p = self.GetActorLocation(); return Math.round(Number(p.X)); }],
    ];

    for (const [label, fn] of tests) {
        try {
            const result = fn();
            console.log(`[diag] ✅ ${label}: ${result}`);
        } catch(e) {
            console.error(`[diag] ❌ ${label}: ${e}`);
        }
    }

    console.log("[diag] --- Done ---");
}
