# Stage 1: Build Wasm and TS packages
FROM emscripten/emsdk:latest AS builder

# Install Node.js and system dependencies
RUN apt-get update && apt-get install -y \
    curl \
    git \
    ninja-build \
    pkg-config \
    zip \
    unzip \
    tar \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y nodejs \
    && npm install -g pnpm

# Set up vcpkg
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && $VCPKG_ROOT/bootstrap-vcpkg.sh

WORKDIR /app

# Copy dependency files for caching
COPY package.json pnpm-lock.yaml pnpm-workspace.yaml ./
COPY packages/core/package.json ./packages/core/
COPY packages/playground/package.json ./packages/playground/
COPY packages/cloudflare-test/package.json ./packages/cloudflare-test/
COPY packages/visual-test/package.json ./packages/visual-test/

# Install dependencies
RUN pnpm install --frozen-lockfile

# Copy source code
COPY . .

# Build Wasm
RUN pnpm wasm:configure
RUN pnpm wasm:build

# Build TypeScript packages
RUN pnpm run build

# Stage 2: Minimal runtime image (for playground)
FROM node:20-slim

WORKDIR /app
RUN npm install -g pnpm

# Copy built artifacts
COPY --from=builder /app /app

EXPOSE 5173

# Default to running the web test environment
CMD ["pnpm", "dev"]
