#!/usr/bin/env node

import { spawnSync } from "node:child_process";
import { existsSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const packageRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const buildDirectory = join(packageRoot, "build");
const compiler = join(buildDirectory, process.platform === "win32" ? "epp.exe" : "epp");

function run(command, args) {
  const result = spawnSync(command, args, { stdio: "inherit" });
  if (result.error) {
    console.error(`e++: ${result.error.message}`);
    process.exit(1);
  }
  if (result.status !== 0) process.exit(result.status ?? 1);
}

function ensureCompiler() {
  if (existsSync(compiler)) return compiler;
  run("cmake", ["-S", packageRoot, "-B", buildDirectory]);
  run("cmake", ["--build", buildDirectory]);
  return compiler;
}

const args = process.argv.slice(2);
if (args.length === 0 || args.includes("--help") || args.includes("-h")) {
  console.log(`Usage:
  e++ run <file.epp>
  e++ <file.epp>

GUI commands are provided by the optional eplusplus-gui package.`);
} else if (args[0] === "build" || args[0] === "serve") {
  console.error(`e++: '${args[0]}' belongs to the optional GUI package. Install it with: npm install -g eplusplus-gui`);
  process.exit(1);
} else {
  run(ensureCompiler(), args[0] === "run" ? args.slice(1) : args);
}
