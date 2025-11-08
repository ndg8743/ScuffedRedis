import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import cors from 'cors';
import { config } from 'dotenv';
import {
  setupRedis,
  getRedisType,
  redisGet,
  redisSet,
  redisDel,
  redisExists,
  redisKeys,
  redisFlushdb,
  redisInfo,
  redisDbsize,
  redisPing
} from './redis';
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

// Execute Redis command
app.post('/execute', async (req, res) => {
  const { command } = req.body;

  if (!command) {
    return res.status(400).json({ error: 'No command provided' });
  }

  try {
    // Parse the command
    const parts = command.trim().split(/\s+/);
    const cmd = parts[0].toUpperCase();
    const args = parts.slice(1);

    let result;

    switch (cmd) {
      case 'PING':
        result = await redisPing();
        break;

      case 'GET':
        if (args.length !== 1) {
          throw new Error('GET requires exactly 1 argument');
        }
        result = await redisGet(args[0]);
        break;

      case 'SET':
        if (args.length < 2) {
          throw new Error('SET requires at least 2 arguments');
        }
        // Handle optional EX (expiry) parameter
        let ttl;
        if (args[2] === 'EX' && args[3]) {
          ttl = parseInt(args[3]);
        }
        await redisSet(args[0], args.slice(1, args[2] === 'EX' ? 2 : undefined).join(' '), ttl);
        result = 'OK';
        break;

      case 'DEL':
        if (args.length < 1) {
          throw new Error('DEL requires at least 1 argument');
        }
        // For simplicity, handle single key deletion
        result = await redisDel(args[0]);
        break;

      case 'EXISTS':
        if (args.length !== 1) {
          throw new Error('EXISTS requires exactly 1 argument');
        }
        result = await redisExists(args[0]);
        break;

      case 'KEYS':
        if (args.length !== 1) {
          throw new Error('KEYS requires exactly 1 argument');
        }
        result = await redisKeys(args[0]);
        break;

      case 'FLUSHDB':
        await redisFlushdb();
        result = 'OK';
        break;

      case 'INFO':
        result = await redisInfo();
        break;

      case 'DBSIZE':
        result = await redisDbsize();
        break;

      case 'ZADD':
        // TODO: Implement sorted set commands
        throw new Error('Sorted set commands not yet implemented in web interface');

      default:
        throw new Error(`Unknown command: ${cmd}`);
    }

    res.json({ result, command });
  } catch (error) {
    res.json({
      error: (error as Error).message,
      command
    });
  }
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
