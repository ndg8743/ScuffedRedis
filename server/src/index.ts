import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import cors from 'cors';
import { config } from 'dotenv';
import { setupRedis, getRedisType } from './redis';
import { setupCache } from './cache';
import { startTrafficGenerator } from './traffic';

// Load environment variables
config({ path: './env' });

const app = express();
const server = createServer(app);
const io = new Server(server, {
  cors: {
    origin: "http://localhost:3000",
    methods: ["GET", "POST"]
  }
});

const PORT = process.env.PORT || 4000;

// Middleware
app.use(cors());
app.use(express.json());

// Health check
app.get('/health', (req, res) => {
  res.json({ ok: true });
});

// Warm up cache with N items
app.post('/warmup', async (req, res) => {
  const count = parseInt(req.query.count as string) || 50;
  const { getItem } = await setupCache();
  
  const warmupPromises = [];
  for (let i = 0; i < count; i++) {
    warmupPromises.push(getItem(i + 1));
  }
  
  await Promise.all(warmupPromises);
  res.json({ warmed: count });
});

// Get hit ratio stats
app.get('/hitratio', (req, res) => {
  const stats = global.cacheStats || { hits: 0, misses: 0, ratio: 0 };
  res.json(stats);
});

io.on('connection', (socket) => {
  console.log('Client connected:', socket.id);
  
  socket.on('disconnect', () => {
    console.log('Client disconnected:', socket.id);
  });
});

global.cacheStats = { hits: 0, misses: 0, ratio: 0 };

async function startServer() {
  try {
    await setupRedis();
    const { getItem } = await setupCache();
    
    startTrafficGenerator(getItem, io);
    
    server.listen(PORT, () => {
      console.log(`Server running on port ${PORT}`);
      console.log(`Connected to: ${getRedisType()}`);
      console.log(`Frontend: http://localhost:3000`);
      console.log(`Backend API: http://localhost:${PORT}`);
    });
  } catch (error) {
    console.error('Failed to start server:', error);
    process.exit(1);
  }
}

startServer();
