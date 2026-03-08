import { execSync } from "child_process";
import fs from "fs";
import path from "path";

const SRC_DIR = "src";
const EXTENSIONS = [".cpp", ".h"];
const IGNORE_DIR = "libs";

function getFiles(dir: string): string[] {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  const files = entries
    .filter((e) => !e.isDirectory() && EXTENSIONS.includes(path.extname(e.name)))
    .map((e) => path.join(dir, e.name));

  const directories = entries
    .filter((e) => e.isDirectory() && e.name !== IGNORE_DIR)
    .map((e) => getFiles(path.join(dir, e.name)));

  return files.concat(...directories);
}

try {
  const files = getFiles(SRC_DIR);
  if (files.length === 0) {
    console.log("No files to format.");
    process.exit(0);
  }

  console.log(`Formatting ${files.length} files...`);
  // Join files with spaces and handle potential spaces in filenames by wrapping in quotes
  const filesArg = files.map(f => `"${f}"`).join(" ");
  execSync(`npx clang-format -i ${filesArg}`, { stdio: "inherit" });
  console.log("Done.");
} catch (error) {
  console.error("Formatting failed:", error);
  process.exit(1);
}
