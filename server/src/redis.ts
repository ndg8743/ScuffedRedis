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
      // Build binary protocol message for GET command
      // Array with ["GET", key]
      const cmd = Buffer.concat([
        Buffer.from([0x05]), // ARRAY type
        Buffer.from([0x00, 0x00, 0x00, 0x02]), // Length 2 (2 elements)
        Buffer.from([0x04]), // BULK_STRING type for "GET"
        Buffer.from([0x00, 0x00, 0x00, 0x03]), // Length 3
        Buffer.from('GET'),
        Buffer.from([0x04]), // BULK_STRING type for key
        Buffer.from([0x00, 0x00, 0x00, key.length]), // Key length
        Buffer.from(key)
      ]);
      
      this.socket!.write(cmd);
      
      this.socket!.once('data', (data) => {
        // Parse binary response
        if (data.length < 5) {
          resolve(null);
          return;
        }
        
        const type = data[0];
        const length = data.readUInt32BE(1);
        
        if (type === 0x06) { // NULL_VALUE
          resolve(null);
        } else if (type === 0x04 || type === 0x01) { // BULK_STRING or SIMPLE_STRING
          const value = data.slice(5, 5 + length).toString();
          resolve(value);
        } else {
          resolve(null);
        }
      });
    });
  }

  async set(key: string, value: string, ttl?: number): Promise<void> {
    if (!this.socket) throw new Error('Not connected');
    
    return new Promise((resolve) => {
      // Build binary protocol message for SET command
      // Array with ["SET", key, value] or ["SET", key, value, "EX", ttl]
      const parts = [];
      const cmdParts = ['SET', key, value];
      if (ttl) {
        cmdParts.push('EX', ttl.toString());
      }
      
      // Build array header
      parts.push(Buffer.from([0x05])); // ARRAY type
      parts.push(Buffer.from([0x00, 0x00, 0x00, cmdParts.length])); // Number of elements
      
      // Add each part as BULK_STRING
      for (const part of cmdParts) {
        const partStr = String(part);
        parts.push(Buffer.from([0x04])); // BULK_STRING type
        const lengthBuf = Buffer.allocUnsafe(4);
        lengthBuf.writeUInt32BE(partStr.length, 0);
        parts.push(lengthBuf);
        parts.push(Buffer.from(partStr));
      }
      
      const cmd = Buffer.concat(parts);
      this.socket!.write(cmd);
      
      this.socket!.once('data', () => {
        resolve();
      });
    });
  }

  async ping(): Promise<string> {
    if (!this.socket) throw new Error('Not connected');
    
    return new Promise((resolve) => {
      // Build binary protocol message for PING command
      // Array with ["PING"]
      const cmd = Buffer.concat([
        Buffer.from([0x05]), // ARRAY type
        Buffer.from([0x00, 0x00, 0x00, 0x01]), // Length 1 (1 element)
        Buffer.from([0x04]), // BULK_STRING type for "PING"
        Buffer.from([0x00, 0x00, 0x00, 0x04]), // Length 4
        Buffer.from('PING')
      ]);
      
      this.socket!.write(cmd);
      
      this.socket!.once('data', (data) => {
        // Parse binary response - expect SIMPLE_STRING "PONG"
        if (data.length >= 5) {
          const type = data[0];
          const length = data.readUInt32BE(1);
          if (type === 0x01) { // SIMPLE_STRING
            const value = data.slice(5, 5 + length).toString();
            resolve(value);
            return;
          }
        }
        resolve('PONG');
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
