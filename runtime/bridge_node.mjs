import fs from "node:fs";
import path from "node:path";
import { createRequire } from "node:module";

const [, , target, fn, reqPath, resPath] = process.argv;
const require = createRequire(import.meta.url);

async function loadModule(name) {
  const searchRoot = process.env.SHARP_NODE_PATH || "";
  if (searchRoot) {
    try {
      const scopedRequire = createRequire(path.join(searchRoot, "__sharp__.cjs"));
      return scopedRequire(name);
    } catch {}
  }
  try {
    return require(name);
  } catch {
    return await import(name);
  }
}

async function main() {
  try {
    const payload = fs.readFileSync(reqPath, "utf8");
    const mod = await loadModule(target);
    const candidate =
      mod?.[fn] ??
      mod?.default?.[fn] ??
      (fn === "default" ? mod?.default ?? mod : undefined);

    if (typeof candidate !== "function") {
      throw new Error(`node bridge: function '${fn}' was not found in '${target}'`);
    }

    const arg = payload.length ? JSON.parse(payload) : null;
    const result = await candidate(arg);
    fs.writeFileSync(resPath, typeof result === "string" ? result : JSON.stringify(result));
  } catch (error) {
    fs.writeFileSync(resPath, String(error?.message ?? error));
    process.exit(1);
  }
}

await main();
