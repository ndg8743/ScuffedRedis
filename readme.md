# Redis Cache Heatmap

A Redis implementation in C++ with 3D real-time cache visualization. Built following the [Build Your Own Redis](https://build-your-own.org/redis/) tutorial.

## Overview

This project demonstrates cache behavior through a 3D voxel wall visualization:
- **Red cubes** indicate cache misses (slow database queries)
- **Green cubes** indicate cache hits (fast lookups)
- **Gray cubes** represent idle state

The visualization shows realistic cache warm-up patterns as the system transitions from mostly misses to mostly hits.

## Architecture

**C++ Redis Server**: Tutorial-based implementation with simple protocol (4-byte length + text commands)  
**Node.js Backend**: Express server with Socket.IO, generates traffic using Zipf distribution  
**React Frontend**: Next.js with React Three Fiber for 3D visualization  
**Production**: Docker container with nginx, SSL, deployed at gopee.dev/scuffedredis

## Quick Start

### Development

Prerequisites: Node.js 18+, CMake, C++ compiler

```bash
# Install dependencies
npm run install:all

# Start all services (C++ server + Node.js backend + React frontend)
npm run dev
```

Access at `http://localhost:3000`

### Production Deployment

```bash
# Deploy to gopee.dev/scuffedredis
npm run deploy
```

## How It Works

**Cache Pattern**: Cache-aside with 60s TTL and simulated database queries  
**Traffic Generation**: Zipf distribution simulating realistic access patterns (80/20 rule)  
**Visualization**: 20x20 grid (400 cubes) with deterministic mapping and real-time WebSocket updates  
**Protocol**: Simple text commands (GET, SET, PING) with 4-byte length prefixes

## Protocol

The C++ server implements a simplified Redis protocol:

```
Format: [4-byte length][command text]
Commands: "GET key", "SET key value", "PING"
```

## Development

```bash
npm run build     # Build C++ server only
npm run dev       # Start all services for development
npm run deploy    # Deploy to production
```

## References

Built following [Build Your Own Redis](https://build-your-own.org/redis/) by James Smith

## License

MIT License