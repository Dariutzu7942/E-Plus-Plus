#!/usr/bin/env node

import { spawnSync, spawn } from "node:child_process";
import { createServer } from "node:http";
import { existsSync, readFileSync, statSync } from "node:fs";
import { dirname, extname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const packageRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const buildDirectory = join(packageRoot, "build");
const compiler = join(buildDirectory, process.platform === "win32" ? "epp.exe" : "epp");

function fail(message) {
  console.error(`e++: ${message}`);
  process.exit(1);
}

function buildCompiler() {
  const cmakeAvailable = spawnSync("cmake", ["--version"], { stdio: "ignore" }).status === 0;
  if (!cmakeAvailable) fail("CMake and a C++17 compiler are required");

  const result = spawnSync("cmake", ["-S", packageRoot, "-B", buildDirectory], { stdio: "inherit" });
  if (result.status !== 0) process.exit(result.status ?? 1);

  const build = spawnSync("cmake", ["--build", buildDirectory], { stdio: "inherit" });
  if (build.status !== 0) process.exit(build.status ?? 1);
}

function getCompiler() {
  if (!existsSync(compiler)) buildCompiler();
  return compiler;
}

function printHelp() {
  console.log(`Usage:
  e++ build <file.epp> [--out <file.html>]
  e++ run <file.epp>
  e++ serve [directory] [--port <number>]
  e++ <file.epp> [--out <file.html>]`);
}

function compileFile(args) {
  const compilerPath = getCompiler();
  const result = spawnSync(compilerPath, args, { stdio: "inherit" });
  if (result.status !== 0) process.exit(result.status ?? 1);
}

function serve(directory, port) {
  const root = resolve(directory);
  const server = createServer((request, response) => {
    const pathname = decodeURIComponent(new URL(request.url ?? "/", "http://localhost").pathname);
    const requestedPath = pathname === "/" ? join(root, "index.epp") : join(root, pathname.replace(/^\/+/, ""));

    let sourcePath = requestedPath;
    if (!existsSync(sourcePath) && extname(sourcePath) === "") sourcePath += ".epp";

    if (!existsSync(sourcePath)) {
      response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
      response.end("404 Not found");
      return;
    }

    if (statSync(sourcePath).isDirectory()) sourcePath = join(sourcePath, "index.epp");

    if (extname(sourcePath) !== ".epp") {
      response.writeHead(200, { "Content-Type": "application/octet-stream" });
      response.end(readFileSync(sourcePath));
      return;
    }

    const outputPath = join(buildDirectory, "served.html");
    const result = spawnSync(getCompiler(), [sourcePath, "--out", outputPath], { stdio: "inherit" });
    if (result.status !== 0) {
      response.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
      response.end("E++ compilation failed");
      return;
    }

    response.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    response.end(readFileSync(outputPath));
  });

  server.listen(port, "127.0.0.1", () => {
    console.log(`E++ server running at http://localhost:${port}`);
  });
}

const args = process.argv.slice(2);
if (args.length === 0 || args.includes("--help") || args.includes("-h")) {
  printHelp();
} else if (args[0] === "build") {
  compileFile(args.slice(1));
} else if (args[0] === "run") {
  compileFile(args);
} else if (args[0] === "serve") {
  let directory = ".";
  let port = 3000;
  for (let index = 1; index < args.length; index++) {
    if (args[index] === "--port") port = Number(args[++index]);
    else if (!args[index].startsWith("-")) directory = args[index];
  }
  serve(directory, port);
} else {
  compileFile(args);
}
