#!/usr/bin/env node

import { spawnSync } from "node:child_process";
import { createServer } from "node:http";
import { existsSync, readFileSync, statSync } from "node:fs";
import { dirname, extname, join, relative, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";

const packageRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const buildDirectory = join(packageRoot, "build");
const compiler = join(buildDirectory, process.platform === "win32" ? "epp-gui.exe" : "epp-gui");

function run(command, args) {
  const result = spawnSync(command, args, { stdio: "inherit" });
  if (result.error) {
    console.error(`e++-gui: ${result.error.message}`);
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

function help() {
  console.log(`Usage:
  e++-gui build <file.epp> [--out <file.html>]
  e++-gui serve [directory] [--port <number>]`);
}

function serve(directory, port) {
  const root = resolve(directory);
  const server = createServer((request, response) => {
    let pathname;
    try {
      pathname = decodeURIComponent(new URL(request.url ?? "/", "http://localhost").pathname);
    } catch {
      response.writeHead(400).end("Bad request");
      return;
    }

    const relativePath = pathname === "/" ? "index.epp" : pathname.replace(/^\/+/, "");
    let sourcePath = resolve(root, relativePath);
    if (sourcePath !== root && !sourcePath.startsWith(root + sep)) {
      response.writeHead(403).end("Forbidden");
      return;
    }
    if (!existsSync(sourcePath) && extname(sourcePath) === "") sourcePath += ".epp";
    if (!existsSync(sourcePath)) {
      response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" }).end("Not found");
      return;
    }
    if (statSync(sourcePath).isDirectory()) sourcePath = join(sourcePath, "index.epp");

    if (extname(sourcePath).toLowerCase() !== ".epp") {
      response.writeHead(200, { "Content-Type": "application/octet-stream" });
      response.end(readFileSync(sourcePath));
      return;
    }

    const outputPath = join(buildDirectory, "served.html");
    const result = spawnSync(ensureCompiler(), [sourcePath, "--out", outputPath], { encoding: "utf8" });
    if (result.status !== 0) {
      response.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
      response.end(result.stderr || "GUI compilation failed");
      return;
    }
    response.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    response.end(readFileSync(outputPath));
  });
  server.listen(port, "127.0.0.1", () => console.log(`E++ GUI server running at http://localhost:${port}`));
}

const args = process.argv.slice(2);
if (args.length === 0 || args.includes("--help") || args.includes("-h")) {
  help();
} else if (args[0] === "build") {
  if (!args[1]) {
    console.error("e++-gui: build requires a .epp file");
    process.exit(1);
  }
  run(ensureCompiler(), args.slice(1));
} else if (args[0] === "serve") {
  let directory = ".";
  let port = 3000;
  for (let index = 1; index < args.length; index += 1) {
    if (args[index] === "--port") port = Number(args[++index]);
    else if (!args[index].startsWith("-")) directory = args[index];
  }
  if (!Number.isInteger(port) || port < 1 || port > 65535) {
    console.error("e++-gui: --port must be a valid TCP port");
    process.exit(1);
  }
  serve(directory, port);
} else {
  console.error(`e++-gui: unknown command '${args[0]}'`);
  help();
  process.exit(1);
}
