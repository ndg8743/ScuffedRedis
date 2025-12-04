# Redis Cache Heatmap

A Redis implementation in C++ with a 3D real-time cache visualization system. Built following the [Build Your Own Redis](https://build-your-own.org/redis/) tutorial by James Smith.

## Overview

This project demonstrates cache warm-up patterns through a 3D voxel wall where:
- **Red cubes** = Cache misses (slow database queries)
- **Green cubes** = Cache hits (fast Redis lookups)
- **Gray cubes** = Idle state

## Architecture

The system consists of three main components:

1. **C++ Redis Server** - Custom Redis implementation using binary protocol on port 6379
2. **Node.js Middleware** - Bridges between C++ server and frontend via WebSocket
3. **React Frontend** - 3D visualization using Three.js

### Components

**Frontend:**
- Next.js 14 with App Router
- React Three Fiber for 3D rendering
- TailwindCSS + shadcn/ui for styling
- Socket.IO for real-time updates

**Backend:**
- Node.js + Express middleware server
- Custom binary protocol client for C++ Redis server
- Socket.IO for WebSocket communication

**C++ Server:**
- Custom Redis implementation
- Binary protocol compatible with Redis RESP
- Hash table and sorted set data structures
- TTL support for cache expiration

## Quick Start

### Prerequisites
- CMake 3.10+
- C++17 compiler
- Node.js 18+
- npm or pnpm

### Build and Run
```bash
npm run install:all
```

### 2. Build and Start All Services
```bash
npm run dev
```

This starts:
- C++ Redis server on port 6379
- Node.js middleware on port 4000
- React frontend on port 3000

### 3. Open the Application
Navigate to `http://localhost:3000`

## Dependencies

This project uses the following main dependencies:

- **React:** 18.2.0
- **Next.js:** 14.2.33
- **Three.js:** 0.159.0
- **ioredis:** 5.3.2

**Note on `three-mesh-bvh`:**

There is a known issue with `three-mesh-bvh` where it may try to import `BatchedMesh` from `three`, which is not available in all versions. This can cause build errors. To resolve this, make sure you are using compatible versions of `three`, `@react-three/drei`, and `@react-three/fiber`.

## How It Works

### Cache-Aside Pattern
The backend implements a cache-aside pattern:
1. **Cache Hit**: Data retrieved from Redis (fast, green flash)
2. **Cache Miss**: Simulated database query (100-300ms delay, red flash)
3. **TTL**: Cached items expire after 60 seconds

### Traffic Generation
- Zipf distribution for realistic access patterns
- 200 unique items with skewed popularity
- 300-500ms intervals between requests

### 3D Visualization
- 20×20 voxel wall (400 cubes total)
- Deterministic mapping: `cellIndex = hash(id) % 400`
- Real-time updates via WebSocket

## Features

- Live cache hit/miss visualization
- Interactive warm-up controls
- Real-time hit ratio statistics
- Optimized 3D rendering with InstancedMesh

## Configuration

### Environment Variables

**Backend** (`server/.env`):
```env
PORT=4000
# Set to true to use the custom C++ Redis server, or false to use a standard Redis instance
USE_SCUFFED_REDIS=true
```

**Frontend** (`web/.env.local`):
```env
NEXT_PUBLIC_SERVER_URL=http://localhost:4000
```

## Project Structure

```
├── src/                   # C++ Redis server
│   ├── server/           # Main server implementation
│   ├── protocol/         # Binary protocol handling
│   ├── data/             # Data structures (hashtable, sorted set)
│   ├── network/          # TCP server and client
│   └── utils/            # Logging utilities
├── server/               # Node.js middleware
│   ├── src/
│   │   ├── index.ts     # Express + Socket.IO server
│   │   ├── cache.ts     # Cache-aside implementation
│   │   ├── redis.ts     # Binary protocol client
│   │   └── traffic.ts   # Traffic generator
│   └── package.json
├── web/                 # React frontend
│   ├── components/      # UI components and 3D scene
│   ├── lib/            # Configuration and utilities
│   └── package.json
└── CMakeLists.txt      # C++ build configuration
```

## Binary Protocol

The C++ server implements a custom binary protocol:

```
Format: [Type:1 byte][Length:4 bytes LE][Data:N bytes]
Types: 0x01=SimpleString, 0x02=Error, 0x03=Integer,
       0x04=BulkString, 0x05=Array, 0x06=Null
```

## Development

### Building the C++ Server
```bash
npm run build:cpp
```

### Running Individual Components
```bash
# C++ server only
npm run dev:cpp

# Node.js middleware only
npm run dev:server

# React frontend only
npm run dev:web
```

## Troubleshooting

### Connection Issues
Check if the C++ server is running:
```bash
# Test connection
telnet localhost 6379
```

### Build Issues
```bash
# Clean and rebuild
rm -rf build/
npm run build:cpp
```

## References

This project is inspired by and follows concepts from:
- [Build Your Own Redis](https://build-your-own.org/redis/) by James Smith
- Redis Protocol Specification (RESP)
- Modern C++ network programming patterns

## License

MIT License - see LICENSE file for details.
