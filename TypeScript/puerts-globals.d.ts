// puerts-globals.d.ts
// Ambient declarations for globals available in the Puerts QuickJS runtime.

declare const puerts: {
    argv: {
        getByName(name: string): any;
    };
};

declare const console: {
    log(...args: any[]): void;
    warn(...args: any[]): void;
    error(...args: any[]): void;
};

declare function setInterval(fn: () => void, ms: number): any;
declare function clearInterval(id: any): void;
declare function setTimeout(fn: () => void, ms: number): any;
declare function clearTimeout(id: any): void;
