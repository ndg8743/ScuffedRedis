import { Server } from 'socket.io';

interface CacheResult {
  id: number;
  value: string;
  hit: boolean;
  latency_ms: number;
}

interface TrafficConfig {
  rate: number;
  pattern: 'constant' | 'spike' | 'wave' | 'random';
  operationType: 'read' | 'write' | 'mixed';
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

// Global traffic config
let currentTrafficConfig: TrafficConfig = {
  rate: 10,
  pattern: 'constant',
  operationType: 'mixed'
};

export function setTrafficConfig(config: Partial<TrafficConfig>) {
  currentTrafficConfig = { ...currentTrafficConfig, ...config };
}

function getOperationType(): 'GET' | 'SET' | 'DEL' {
  const opType = currentTrafficConfig.operationType;
  const rand = Math.random();

  if (opType === 'read') return 'GET';
  if (opType === 'write') return 'SET';

  if (rand < 0.7) return 'GET';
  if (rand < 0.9) return 'SET';
  return 'DEL';
}

function calculateInterval(): number {
  const baseRate = currentTrafficConfig.rate;
  const pattern = currentTrafficConfig.pattern;

  switch (pattern) {
    case 'constant':
      return 1000 / baseRate;

    case 'spike':
      return Math.random() < 0.1 ? 50 : 1000 / baseRate;

    case 'wave':
      const wave = Math.sin(Date.now() / 1000) * 0.5 + 0.5;
      return 1000 / (baseRate * wave + baseRate * 0.5);

    case 'random':
      return Math.random() * (2000 / baseRate);

    default:
      return 1000 / baseRate;
  }
}

export function startTrafficGenerator(
  getItem: (id: number) => Promise<CacheResult>,
  io: Server
) {
  const zipf = new ZipfDistribution(200, 1.2);

  async function generateTraffic() {
    const interval = setInterval(async () => {
      try {
        const itemId = zipf.getRandomItem();
        const operationType = getOperationType();
        const result = await getItem(itemId);

        // Emit with operation type and enhanced data
        io.emit('cache_event', {
          type: 'cache_event',
          id: result.id,
          hit: result.hit,
          latency_ms: result.latency_ms,
          operationType,
          timestamp: Date.now(),
          cacheStats: (global as any).cacheStats || { hits: 0, misses: 0, ratio: 0 }
        });

      } catch (error) {
        console.error('Error in traffic generator:', error);
      }
    }, calculateInterval());

    (global as any).trafficInterval = interval;
  }

  generateTraffic();
  console.log('Traffic generator started with config:', currentTrafficConfig);
}

export function stopTrafficGenerator() {
  const interval = (global as any).trafficInterval;
  if (interval) {
    clearInterval(interval);
    (global as any).trafficInterval = null;
    console.log('Traffic generator stopped');
  }
}





