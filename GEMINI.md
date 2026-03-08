# GEMINI.md

## 1. Operational Guidelines (Agent Specific)

This section defines the core rules for an agent's behavior within this project.

### 1.1 Shell Environment: Windows PowerShell

- **Command Chaining**: Use `;` for sequential execution (do NOT use `&&` or `||`).
- **Constraint**: Do not execute multiple shell commands in parallel.

### 1.2 Communication & Troubleshooting

- **Progress Reporting**: Always report your current status in the user's language concisely before and during each step.
- **Hash Mismatch**: If encountered, immediately re-read the file with `get_text_file_contents` to synchronize. If the issue persists, overwrite the entire file using `write_file`.

### 1.3 Git Usage

- **No Write Operations**: Do not perform `git add`, `commit`, `push`, etc., unless explicitly and specifically instructed by the user.
- **Read-Only Access**: `status`, `diff`, and `log` are permitted and recommended for gathering context.

### 1.4 Serena Tool Constraints

- **execute_shell_command**: Use of `execute_shell_command` is strictly prohibited. Use `run_shell_command` instead.
- **serena\_\_read_file**: Use of `serena__read_file` is strictly prohibited. Use `read_text_file` or other alternative tools instead.
- **replace_content**: Use of `replace_content` is strictly prohibited. Use `edit_file` or other alternative tools instead.
- **find_symbol**: Use of `find_symbol` is strictly prohibited. Use `get_symbols_overview` and `read_text_file` with line numbers or other alternative tools instead.

---

## 2. Project Overview: Image Converter

A high-fidelity image converter (Wasm) supporting multiple formats and animations.

- **Core Technologies**: `Skia` (Rendering & Encoding) + `libavif` (AVIF Decoding/Encoding via `libaom` & `dav1d`) + `wuffs` (GIF Decoding) + `expat` (SVG Parsing).
- **Structure**: pnpm workspaces monorepo.
  - `packages/core`: Core library & TS wrappers.
    - `core.ts`: Shared logic & base class.
    - `index.ts`: Basic API.
    - `workers.ts` & `child-workers.ts`: Multi-threaded rendering via Worker Threads.
    - `single.ts`: Bundled Wasm support.
    - `workerd.ts`: Cloudflare Workers (workerd) support.
  - `packages/playground`: Web-based demo application.

---

## 3. Technical Specifications & Architecture

### 3.1 Directory Structure

- `src/cpp/api`: Emscripten API implementation (`image_converter_api.cpp`).
- `src/cpp/bridge`: Unified type definitions shared across modules (`bridge_types.h`).
- `src/cpp/core`: Global context and resource management (`image_converter_context.cpp`).
- `src/cpp/libs`: External libraries and configurations (`skia`, `libavif`).
- `src/cpp/utils`: Helper utilities and logging.

### 3.2 Image Processing Pipeline

- **Multi-Frame Architecture**: `ImageConverterInstance` manages a `std::vector<ImageFrame>`, allowing for animation support (GIF, WebP).
- **Unified Loading Pipeline**:
  - **Raster Images**: Uses `SkCodec` with explicit registration of PNG, JPEG, WebP, GIF (via Wuffs), AVIF, and BMP decoders.
  - **SVG**: High-fidelity SVG rendering via `SkSVGDOM`.
- **Processing**:
  - **Resize**: Linear interpolation based resizing with `FitMode` support (`Contain`, `Cover`, `Fill`).
  - **Animation Handling**: Automatically forces output to WebP if `animation` is true and source is an animation.
  - **Metadata Extraction**: Supports retrieving original size, format, and animation status.
- **Encoding Pipeline**:
  - **Still Images**: High-quality encoding for PNG, JPEG, WebP, and AVIF.
  - **Animations**: Support for animated WebP encoding using `SkWebpEncoder::EncodeAnimated` (controllable via `animation` parameter).
  - **Transparency**: Supports `None` format to bypass encoding and return original data with metadata.
  - **Optimization**: Configurable AVIF encoding speed (0-10).

### 3.3 Development Environment

- **C++ Standard**: C++20 (required by modern Skia modules).
- **Wasm/Emscripten**: Optimized with `-Oz`, `-flto=thin`, and SIMD support (`-msimd128`).
- **Dependency Management**: `vcpkg` for managing `libpng`, `libjpeg-turbo`, `libwebp`, `dav1d`, and `expat`.

---

## 4. Development & Verification Workflow

### 4.1 Build Commands

- `pnpm wasm:configure`: Configure CMake with Ninja and vcpkg.
- `pnpm wasm:build`: Compile C++ to Wasm.
- `pnpm build`: Full monorepo build.

### 4.2 Verification Protocol

1. **Build Check**: Ensure Wasm compilation success.
2. **Format Support**: Verify decoding and encoding across all supported formats (including animations).

---
