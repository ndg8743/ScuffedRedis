import Redis from 'ioredis';
import net from 'net';

let redis: Redis | null = null;
let useScuffedRedis = false;

// Simple client for ScuffedRedis C++ server
class ScuffedRedisClient {
  private socket: net.Socket | null = null;
  private host: string;
  private port: number;

  constructor(host: string = 'localhost', port: number = 6379) {
    this.host = host;
    this.port = port;
  }

  async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.socket = new net.Socket();
      
      this.socket.connect(this.port, this.host, () => {
        console.log('✅ Connected to ScuffedRedis C++ server');
        resolve();
      });
      
      this.socket.on('error', (err) => {
        reject(err);
      });
    });
  }

  async get(key: string): Promise<string | null> {
    if (!this.socket) throw new Error('Not connected');
    
    return new Promise((resolve) => {
      // Simple text protocol for now - will be enhanced with binary protocol later
      this.socket!.write(`GET ${key}\r\n`);
      
      this.socket!.once('data', (data) => {
        const response = data.toString().trim();
        // Simple response parsing - enhance this based on ScuffedRedis protocol
        if (response.includes('(nil)') || response.includes('null')) {
          resolve(null);
        } else {
          resolve(response);
        }
      });
    });
  }

  async set(key: string, value: string, ttl?: number): Promise<void> {
    if (!this.socket) throw new Error('Not connected');
    
    return new Promise((resolve) => {
      const cmd = ttl ? `SET ${key} ${value} EX ${ttl}\r\n` : `SET ${key} ${value}\r\n`;
      this.socket!.write(cmd);
      
      this.socket!.once('data', () => {
        resolve();
      });
    });
  }

  async ping(): Promise<string> {
    if (!this.socket) throw new Error('Not connected');
    
    return new Promise((resolve) => {
      this.socket!.write('PING\r\n');
      
      this.socket!.once('data', (data) => {
        resolve(data.toString().trim());
      });
    });
  }

  disconnect(): void {
    if (this.socket) {
      this.socket.destroy();
      this.socket = null;
    }
  }
}

let scuffedRedisClient: ScuffedRedisClient | null = null;

export async function setupRedis(): Promise<void> {
  const redisUrl = process.env.REDIS_URL || 'redis://localhost:6379';
  const useScuffed = process.env.USE_SCUFFED_REDIS === 'true';
  
  if (useScuffed) {
    // Try to connect to ScuffedRedis C++ server
    try {
      scuffedRedisClient = new ScuffedRedisClient('localhost', 6379);
      await scuffedRedisClient.connect();
      useScuffedRedis = true;
      console.log('🔥 Using ScuffedRedis C++ server');
      return;
    } catch (error) {
      console.log('⚠️ ScuffedRedis not available, falling back to standard Redis');
    }
  }
  
  // Fall back to standard Redis
  redis = new Redis(redisUrl, {
    retryDelayOnFailover: 100,
    maxRetriesPerRequest: 3,
    lazyConnect: true
  });

  redis.on('connect', () => {
    console.log('✅ Connected to standard Redis');
  });

  redis.on('error', (err) => {
    console.error('❌ Redis connection error:', err);
  });

  await redis.connect();
  useScuffedRedis = false;
}

export async function redisGet(key: string): Promise<string | null> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.get(key);
  } else if (redis) {
    return await redis.get(key);
  }
  throw new Error('No Redis client available');
}

export async function redisSet(key: string, value: string, ttl?: number): Promise<void> {
  if (useScuffedRedis && scuffedRedisClient) {
    await scuffedRedisClient.set(key, value, ttl);
  } else if (redis) {
    if (ttl) {
      await redis.setex(key, ttl, value);
    } else {
      await redis.set(key, value);
    }
  } else {
    throw new Error('No Redis client available');
  }
}

export async function redisPing(): Promise<string> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.ping();
  } else if (redis) {
    return await redis.ping();
  }
  throw new Error('No Redis client available');
}

export function getRedisType(): string {
  return useScuffedRedis ? 'ScuffedRedis C++' : 'Standard Redis';
}

export async function closeRedis(): Promise<void> {
  if (scuffedRedisClient) {
    scuffedRedisClient.disconnect();
    scuffedRedisClient = null;
  }
  if (redis) {
    await redis.quit();
    redis = null;
  }
}
