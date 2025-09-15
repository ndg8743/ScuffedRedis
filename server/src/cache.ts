import { redisGet, redisSet, getRedisType } from './redis';

interface CacheResult {
  id: number;
  value: string;
  hit: boolean;
  latency_ms: number;
}

// Simulate database delay
function sleep(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// Generate random value for an item
function generateValue(id: number): string {
  return `value_${id}_${Date.now()}`;
}

export async function setupCache() {
  console.log(`🔧 Cache setup using: ${getRedisType()}`);

  async function getItem(id: number): Promise<CacheResult> {
    const startTime = Date.now();
    const key = `item:${id}`;
    
    try {
      // Try to get from cache
      const cached = await redisGet(key);
      
      if (cached) {
        // Cache hit
        const latency = Date.now() - startTime;
        updateStats(true);
        
        return {
          id,
          value: cached,
          hit: true,
          latency_ms: latency
        };
      } else {
        // Cache miss - simulate database query
        const dbDelay = 600 + Math.random() * 600; // 600-1200ms
        await sleep(dbDelay);
        
        const value = generateValue(id);
        
        // Store in cache with 60 second TTL
        await redisSet(key, value, 60);
        
        const latency = Date.now() - startTime;
        updateStats(false);
        
        return {
          id,
          value,
          hit: false,
          latency_ms: latency
        };
      }
    } catch (error) {
      console.error(`Error getting item ${id}:`, error);
      const latency = Date.now() - startTime;
      updateStats(false);
      
      return {
        id,
        value: generateValue(id),
        hit: false,
        latency_ms: latency
      };
    }
  }

  return { getItem };
}

// Update global stats
function updateStats(hit: boolean) {
  if (!global.cacheStats) {
    global.cacheStats = { hits: 0, misses: 0, ratio: 0 };
  }
  
  if (hit) {
    global.cacheStats.hits++;
  } else {
    global.cacheStats.misses++;
  }
  
  const total = global.cacheStats.hits + global.cacheStats.misses;
  global.cacheStats.ratio = total > 0 ? global.cacheStats.hits / total : 0;
}

// Declare global type
declare global {
  var cacheStats: {
    hits: number;
    misses: number;
    ratio: number;
  };
}
