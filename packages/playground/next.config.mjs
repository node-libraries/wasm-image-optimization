const isProd = process.env.NODE_ENV === "production";
const repo = process.env.GITHUB_REPOSITORY;
const basePath = repo ? `/${repo.split("/")[1]}` : "";

/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  output: "export",
  basePath: basePath || undefined,
  assetPrefix: basePath ? `${basePath}/` : undefined,
  env: {
    NEXT_PUBLIC_BASE_PATH: basePath,
  },
};

export default nextConfig;
