import Redis from 'ioredis';
import net from 'net';

let redis: Redis | null = null;
let useScuffedRedis = false;
let useTcpFallback = false;

// Simple in-memory cache for mock mode
const mockCache = new Map<string, { value: string; expires: number }>();

class ScuffedRedisClient {
  private socket: net.Socket | null = null;
  private pendingResponse: Buffer = Buffer.alloc(0);
  private host: string;
  private port: number;

  constructor(host: string = 'localhost', port: number = 6379) {
    this.host = host;
    this.port = port;
  }

  async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.socket = new net.Socket();
      this.socket.setTimeout(1000); // 1 second timeout

      this.socket.connect(this.port, this.host, () => {
        resolve();
      });

      this.socket.on('timeout', () => {
        this.socket?.destroy();
        reject(new Error('Connection timeout'));
      });

      this.socket.on('error', reject);
    });
  }

  private buildCommand(args: string[]): Buffer {
    const buffers: Buffer[] = [];

    // Array header
    buffers.push(Buffer.from([0x05])); // ARRAY type
    const len = Buffer.allocUnsafe(4);
    len.writeUInt32LE(args.length, 0); // Little-endian
    buffers.push(len);

    // Each argument as BULK_STRING
    for (const arg of args) {
      buffers.push(Buffer.from([0x04])); // BULK_STRING type
      const argLen = Buffer.allocUnsafe(4);
      argLen.writeUInt32LE(arg.length, 0); // Little-endian
      buffers.push(argLen);
      buffers.push(Buffer.from(arg));
    }

    return Buffer.concat(buffers);
  }

  private parseResponse(data: Buffer): any {
    if (data.length < 5) return null;

    const type = data[0];
    const length = data.readUInt32LE(1); // Little-endian

    if (type === 0x06) return null; // NULL
    if (type === 0x04 || type === 0x01) { // BULK_STRING or SIMPLE_STRING
      return data.slice(5, 5 + length).toString();
    }

    return null;
  }

  private async sendCommand(args: string[]): Promise<any> {
    if (!this.socket) throw new Error('Not connected');

    return new Promise((resolve) => {
      const cmd = this.buildCommand(args);
      this.socket!.write(cmd);

      this.socket!.once('data', (data) => {
        const result = this.parseResponse(data);
        resolve(result);
      });
    });
  }

  async get(key: string): Promise<string | null> {
    return await this.sendCommand(['GET', key]);
  }

  async set(key: string, value: string, ttl?: number): Promise<void> {
    const args = ['SET', key, value];
    if (ttl) {
      args.push('EX', ttl.toString());
    }
    await this.sendCommand(args);
  }

  async ping(): Promise<string> {
    const result = await this.sendCommand(['PING']);
    return result || 'PONG';
  }

  async del(key: string): Promise<number> {
    const result = await this.sendCommand(['DEL', key]);
    return parseInt(result) || 0;
  }

  async exists(key: string): Promise<number> {
    const result = await this.sendCommand(['EXISTS', key]);
    return parseInt(result) || 0;
  }

  async keys(pattern: string): Promise<string[]> {
    const result = await this.sendCommand(['KEYS', pattern]);
    if (!result) return [];
    return result.split(',').filter((k: string) => k.length > 0);
  }

  async flushdb(): Promise<void> {
    await this.sendCommand(['FLUSHDB']);
  }

  async info(): Promise<string> {
    return await this.sendCommand(['INFO']) || '';
  }

  async dbsize(): Promise<number> {
    const result = await this.sendCommand(['DBSIZE']);
    return parseInt(result) || 0;
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
  const scuffedPort = parseInt(process.env.SCUFFED_REDIS_PORT || '6379');

  // Try ScuffedRedis first if configured
  if (useScuffed) {
    try {
      scuffedRedisClient = new ScuffedRedisClient('localhost', scuffedPort);
      await scuffedRedisClient.connect();
      useScuffedRedis = true;
      console.log(`✅ Connected to ScuffedRedis C++ server on port ${scuffedPort}`);
      return;
    } catch (error) {
      console.log('📡 ScuffedRedis not available, trying standard Redis...');
    }
  }

  // Try standard Redis
  try {
    redis = new Redis(redisUrl, {
      maxRetriesPerRequest: 1,
      lazyConnect: true,
      connectTimeout: 1000,
      retryStrategy: () => null // Don't retry on initial connection
    });

    redis.on('error', () => {
      // Silently handle errors
    });

    await redis.connect();
    console.log('✅ Connected to Redis');
    useScuffedRedis = false;
    useTcpFallback = false;
    return;
  } catch (error) {
    console.log('⚠️ Standard Redis not available');
  }

  // Fallback to mock mode
  console.log('⚠️ No Redis available - running in mock mode (in-memory cache)');
  redis = null;
  useScuffedRedis = false;
  useTcpFallback = false;
}

export async function redisGet(key: string): Promise<string | null> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.get(key);
  } else if (redis) {
    return await redis.get(key);
  }
  // Mock mode - use in-memory cache
  const entry = mockCache.get(key);
  if (entry && entry.expires > Date.now()) {
    return entry.value;
  }
  mockCache.delete(key);
  return null;
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
    // Mock mode - use in-memory cache
    const expires = ttl ? Date.now() + (ttl * 1000) : Date.now() + 3600000;
    mockCache.set(key, { value, expires });
  }
}

export async function redisPing(): Promise<string> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.ping();
  } else if (redis) {
    return await redis.ping();
  }
  return 'PONG';
}

export async function redisDel(key: string): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.del(key);
  } else if (redis) {
    return await redis.del(key);
  }
  // Mock mode
  return mockCache.delete(key) ? 1 : 0;
}

export async function redisExists(key: string): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.exists(key);
  } else if (redis) {
    return await redis.exists(key);
  }
  // Mock mode
  const entry = mockCache.get(key);
  return entry && entry.expires > Date.now() ? 1 : 0;
}

export async function redisKeys(pattern: string): Promise<string[]> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.keys(pattern);
  } else if (redis) {
    return await redis.keys(pattern);
  }
  // Mock mode
  return Array.from(mockCache.keys());
}

export async function redisFlushdb(): Promise<void> {
  if (useScuffedRedis && scuffedRedisClient) {
    await scuffedRedisClient.flushdb();
  } else if (redis) {
    await redis.flushdb();
  } else {
    mockCache.clear();
  }
}

export async function redisInfo(): Promise<string> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.info();
  } else if (redis) {
    return await redis.info();
  }
  return 'mock_mode:true\r\nkeys:' + mockCache.size;
}

export async function redisDbsize(): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.dbsize();
  } else if (redis) {
    return await redis.dbsize();
  }
  return mockCache.size;
}

export function getRedisType(): string {
  if (useScuffedRedis && scuffedRedisClient) {
    return 'ScuffedRedis C++ (Full Implementation)';
  } else if (redis) {
    return 'Standard Redis';
  }
  return 'In-Memory (Mock)';
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
  mockCache.clear();
}
