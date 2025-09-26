# Redis Cache Heatmap

A Redis implementation in C++ with a 3D real-time cache visualization system. Built following the [Build Your Own Redis](https://build-your-own.org/redis/) tutorial by James Smith.

## Overview

This project demonstrates cache warm-up patterns through a 3D voxel wall where:
- **Red cubes** = Cache misses (slow database queries)
- **Green cubes** = Cache hits (fast Redis lookups)
- **Gray cubes** = Idle state

## Architecture

### Frontend
- **Next.js 14** (App Router)
- **React Three Fiber** (R3F) for 3D rendering
- **@react-three/drei** for 3D utilities
- **TailwindCSS** + **shadcn/ui** for styling
- **Zustand** for state management
- **Socket.IO client** for real-time updates

### Backend
- **Node.js** + **Express**
- **Socket.IO** for real-time communication
- **ioredis** for Redis client
- **TypeScript** throughout

## 🚀 Quick Start

### Prerequisites
- Node.js 18+
- Redis server
- pnpm (recommended) or npm

### 1. Install Dependencies
```bash
pnpm install
```

### 2. Start Redis
```bash
# Using Docker (recommended)
docker run -p 6379:6379 --name redis redis:7

# Or using your local Redis installation
redis-server
```

### 3. Start Development Servers
```bash
pnpm dev
```

This will start:
- Backend server on `http://localhost:4000`
- Frontend on `http://localhost:3000`

### 4. Open the Application
Navigate to `http://localhost:3000` and watch the magic happen!

## 📊 How It Works

### Cache-Aside Pattern
The backend implements a cache-aside pattern:
1. **Cache Hit**: Data retrieved from Redis (fast, green flash)
2. **Cache Miss**: Simulated database query (600-1200ms delay, red flash)
3. **TTL**: Cached items expire after 60 seconds

### Traffic Generation
- **Zipf Distribution**: Realistic access patterns (top 20 items are much hotter)
- **200 unique items** with skewed popularity
- **100-200ms intervals** between requests

### 3D Visualization
- **20×20 voxel wall** (400 cubes total)
- **Deterministic mapping**: `cellIndex = hash(id) % 400`
- **Smooth animations**: Color transitions and pulse effects
- **Real-time updates** via Socket.IO

## 🎮 Features

### Real-Time Visualization
- Live cache hit/miss events
- Smooth color transitions
- Pulse animations on cache events
- Auto-rotating camera

### Interactive Controls
- **Warm Up Button**: Pre-populate cache with 50 items
- **Live Hit Ratio**: Real-time statistics
- **Connection Status**: Server connectivity indicator

### Performance Optimized
- **InstancedMesh**: Efficient 3D rendering
- **60fps target**: Smooth animations
- **Low GC**: Minimal object creation per frame

## 🔧 Configuration

### Environment Variables

#### Backend (`server/.env`)
```env
PORT=4000
REDIS_URL=redis://localhost:6379
```

#### Frontend (`web/.env.local`)
```env
NEXT_PUBLIC_SERVER_URL=http://localhost:4000
```

### 3D Scene Settings (`web/lib/config.ts`)
```typescript
export const CONFIG = {
  WALL_ROWS: 20,        // Number of rows
  WALL_COLS: 20,        // Number of columns
  PULSE_DURATION: 900,  // Animation duration (ms)
  // ... more settings
};
```

## 📁 Project Structure

```
├── server/                 # Backend Node.js server
│   ├── src/
│   │   ├── index.ts       # Express + Socket.IO server
│   │   ├── cache.ts       # Cache-aside implementation
│   │   ├── redis.ts       # Redis client setup
│   │   └── traffic.ts     # Traffic generator
│   └── package.json
├── web/                   # Frontend Next.js app
│   ├── app/              # Next.js App Router
│   ├── components/       # React components
│   │   ├── ui/          # shadcn/ui components
│   │   ├── Scene.tsx    # 3D scene wrapper
│   │   ├── Heatmap.tsx  # 3D voxel wall
│   │   ├── TopBar.tsx   # UI controls
│   │   └── Legend.tsx   # Color legend
│   ├── lib/             # Utilities and state
│   └── package.json
├── package.json          # Workspace root
└── pnpm-workspace.yaml
```

## 🎨 Customization

### Colors
Modify colors in `web/lib/config.ts`:
```typescript
COLORS: {
  HIT: [0, 1, 0],      // Green
  MISS: [1, 0, 0],     // Red
  IDLE: [0.2, 0.2, 0.2], // Dark gray
  NEUTRAL: [0.3, 0.3, 0.3] // Medium gray
}
```

### Wall Size
Change the voxel wall dimensions:
```typescript
WALL_ROWS: 20,  // Increase for more rows
WALL_COLS: 20,  // Increase for more columns
```

### Animation Speed
Adjust animation timing:
```typescript
PULSE_DURATION: 900,      // Pulse animation duration
COLOR_FADE_DURATION: 2000, // Color transition duration
```

## 🐛 Troubleshooting

### Redis Connection Issues
```bash
# Check if Redis is running
redis-cli ping

# Should return: PONG
```

### Port Conflicts
- Backend: Change `PORT` in `server/.env`
- Frontend: Change port in `web/package.json` scripts

### Build Issues
```bash
# Clean and reinstall
rm -rf node_modules
rm -rf server/node_modules
rm -rf web/node_modules
pnpm install
```

## 🚀 Production Deployment

### Build for Production
```bash
pnpm build
```

### Start Production Servers
```bash
pnpm start
```

### Environment Setup
- Set `NODE_ENV=production`
- Configure production Redis URL
- Set up proper CORS origins

## 📈 Performance Tips

1. **Redis Optimization**: Use Redis with persistence disabled for better performance
2. **3D Rendering**: Reduce `WALL_ROWS`/`WALL_COLS` for lower-end devices
3. **Network**: Use WebSocket transport for better real-time performance

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

MIT License - feel free to use this project for learning and experimentation!

---

**Enjoy watching your cache warm up! 🔥➡️❄️**