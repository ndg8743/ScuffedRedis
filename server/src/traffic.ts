import { Server } from 'socket.io';

interface CacheResult {
  id: number;
  value: string;
  hit: boolean;
  latency_ms: number;
}

// Zipf-like distribution for realistic cache access patterns
class ZipfDistribution {
  private items: number[];
  private weights: number[];
  private totalWeight: number;

  constructor(maxItems: number = 200, skew: number = 1.2) {
    this.items = Array.from({ length: maxItems }, (_, i) => i + 1);
    this.weights = this.items.map((item, index) => 1 / Math.pow(index + 1, skew));
    this.totalWeight = this.weights.reduce((sum, weight) => sum + weight, 0);
  }

  getRandomItem(): number {
    const random = Math.random() * this.totalWeight;
    let cumulative = 0;
    
    for (let i = 0; i < this.items.length; i++) {
      cumulative += this.weights[i];
      if (random <= cumulative) {
        return this.items[i];
      }
    }
    
    return this.items[this.items.length - 1];
  }
}

export function startTrafficGenerator(
  getItem: (id: number) => Promise<CacheResult>,
  io: Server
) {
  const zipf = new ZipfDistribution(200, 1.2);
  let isRunning = false;

  async function generateTraffic() {
    if (isRunning) return;
    isRunning = true;

    const interval = setInterval(async () => {
      try {
        const itemId = zipf.getRandomItem();
        const result = await getItem(itemId);
        
        // Emit to all connected clients
        io.emit('cache_event', {
          type: 'cache_event',
          id: result.id,
          hit: result.hit,
          latency_ms: result.latency_ms
        });
        
      } catch (error) {
        console.error('Error in traffic generator:', error);
      }
    }, 100 + Math.random() * 100); // 100-200ms intervals

    // Store interval for cleanup
    (global as any).trafficInterval = interval;
  }

  // Start traffic generation
  generateTraffic();
  
  console.log('🚦 Traffic generator started with Zipf distribution');
}

export function stopTrafficGenerator() {
  const interval = (global as any).trafficInterval;
  if (interval) {
    clearInterval(interval);
    (global as any).trafficInterval = null;
    console.log('🛑 Traffic generator stopped');
  }
}
