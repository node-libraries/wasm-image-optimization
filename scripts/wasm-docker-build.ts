import { execSync } from "child_process";
import * as path from "path";
import * as fs from "fs";

const projectRoot = process.cwd();
const distPath = path.join(projectRoot, "packages/core/dist");

if (!fs.existsSync(distPath)) {
  fs.mkdirSync(distPath, { recursive: true });
}

console.log("Building Docker image...");
try {
  // Ensure BuildKit is used for cache mounts
  const env = { ...process.env, DOCKER_BUILDKIT: "1" };
  execSync("docker build -f docker/Dockerfile.wasm -t wasm-image-optimization-wasm-build .", {
    stdio: "inherit",
    env,
  });

  console.log("Extracting build artifacts...");
  const absoluteDistPath = path.resolve(distPath);

  // For Windows, ensure path is compatible with Docker volume mount
  const dockerPath = absoluteDistPath.replace(/\\/g, "/");

  execSync(`docker run --rm -v "${dockerPath}:/output" wasm-image-optimization-wasm-build`, {
    stdio: "inherit",
  });

  console.log("Wasm build via Docker completed successfully!");
} catch (error) {
  console.error("Error during Docker Wasm build:", error);
  process.exit(1);
}