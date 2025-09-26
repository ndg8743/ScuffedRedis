/** @type {import('next').NextConfig} */
const nextConfig = {
  typescript: {
    ignoreBuildErrors: false,
  },
  eslint: {
    ignoreDuringBuilds: false,
  },
  transpilePackages: ['three', '@react-three/fiber', '@react-three/drei'],
  output: 'standalone',
  basePath: '/scuffedredis',
  assetPrefix: '/scuffedredis',
  experimental: {
    outputFileTracingRoot: process.cwd(),
  },
}

module.exports = nextConfig
