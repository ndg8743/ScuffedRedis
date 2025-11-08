import express, { Request, Response } from 'express';
import { createServer } from 'http';
import { Server, Socket } from 'socket.io';
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
app.get('/health', (req: Request, res: Response) => {
  res.json({ ok: true });
});

// Cache getter captured at startup for reuse
let getItemFn: ((id: number) => Promise<any>) | null = null;

// Warm up cache with N items (fire-and-forget to avoid upstream timeouts)
app.post('/warmup', async (req: Request, res: Response) => {
  const count = parseInt((req.query.count as string) || '50', 10);

  // Ensure we have a getter; lazily initialize if missing
  if (!getItemFn) {
    const { getItem } = await setupCache();
    getItemFn = getItem;
  }

  const startedAt = Date.now();
  const ids = Array.from({ length: count }, (_, i) => i + 1);

  // Run in background; log completion
  (async () => {
    try {
      const promises = ids.map((id) => getItemFn!(id));
      await Promise.allSettled(promises);
      const ms = Date.now() - startedAt;
      console.log(`Warmup completed: ${count} items in ${ms}ms`);
    } catch (e) {
      console.error('Warmup error:', e);
    }
  })();

  res.status(202).json({ status: 'accepted', count });
});

// Get hit ratio stats
app.get('/hitratio', (req: Request, res: Response) => {
  const stats = (global as any).cacheStats || { hits: 0, misses: 0, ratio: 0 };
  res.json(stats);
});

io.on('connection', (socket: Socket) => {
  console.log('Client connected:', socket.id);

  socket.on('disconnect', () => {
    console.log('Client disconnected:', socket.id);
  });
});

(global as any).cacheStats = { hits: 0, misses: 0, ratio: 0 };

async function startServer() {
  try {
    await setupRedis();
    const { getItem } = await setupCache();
    getItemFn = getItem;

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
