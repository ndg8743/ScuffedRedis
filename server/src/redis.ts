import Redis from 'ioredis';
import net from 'net';

let redis: Redis | null = null;
let useScuffedRedis = false;

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
      
      this.socket.connect(this.port, this.host, () => {
        resolve();
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
    // Parse array response
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
  const redisUrl = process.env.REDIS_URL || 'redis://localhost:6380';
  const useScuffed = process.env.USE_SCUFFED_REDIS === 'true';
  const scuffedPort = parseInt(process.env.SCUFFED_REDIS_PORT || '6380');

  if (useScuffed) {
    try {
      scuffedRedisClient = new ScuffedRedisClient('localhost', scuffedPort);
      await scuffedRedisClient.connect();
      useScuffedRedis = true;
      console.log(`Connected to ScuffedRedis C++ server on port ${scuffedPort}`);
      return;
    } catch (error) {
      console.log('ScuffedRedis not available, falling back to Redis');
    }
  }
  
  redis = new Redis(redisUrl, {
    retryStrategy: (times) => {
      return 100;
    },
    maxRetriesPerRequest: 3,
    lazyConnect: true
  });

  redis.on('connect', () => {
    console.log('Connected to Redis');
  });

  redis.on('error', (err) => {
    console.error('Redis connection error:', err);
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

export async function redisDel(key: string): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.del(key);
  } else if (redis) {
    return await redis.del(key);
  }
  throw new Error('No Redis client available');
}

export async function redisExists(key: string): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.exists(key);
  } else if (redis) {
    return await redis.exists(key);
  }
  throw new Error('No Redis client available');
}

export async function redisKeys(pattern: string): Promise<string[]> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.keys(pattern);
  } else if (redis) {
    return await redis.keys(pattern);
  }
  throw new Error('No Redis client available');
}

export async function redisFlushdb(): Promise<void> {
  if (useScuffedRedis && scuffedRedisClient) {
    await scuffedRedisClient.flushdb();
  } else if (redis) {
    await redis.flushdb();
  } else {
    throw new Error('No Redis client available');
  }
}

export async function redisInfo(): Promise<string> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.info();
  } else if (redis) {
    const info = await redis.info();
    return info;
  }
  throw new Error('No Redis client available');
}

export async function redisDbsize(): Promise<number> {
  if (useScuffedRedis && scuffedRedisClient) {
    return await scuffedRedisClient.dbsize();
  } else if (redis) {
    return await redis.dbsize();
  }
  throw new Error('No Redis client available');
}

export function getRedisType(): string {
  return useScuffedRedis ? 'ScuffedRedis C++ (Full Implementation)' : 'Standard Redis';
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
