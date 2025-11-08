import { create } from 'zustand';

export type CommandType = 'GET' | 'SET' | 'DEL' | 'INCR' | 'EXPIRE' | 'OTHER';

export interface CacheEvent {
  id: number;
  hit: boolean;
  latency_ms: number;
  timestamp: number;
  command?: CommandType;
  key?: string;
}

export interface HitRatioStats {
  hits: number;
  misses: number;
  ratio: number;
}

export interface CommandStats {
  GET: number;
  SET: number;
  DEL: number;
  INCR: number;
  EXPIRE: number;
  OTHER: number;
}

interface AppState {
  // Cache events
  events: CacheEvent[];
  addEvent: (event: Omit<CacheEvent, 'timestamp'>) => void;

  // Hit ratio stats
  hitRatio: HitRatioStats;
  updateHitRatio: (stats: HitRatioStats) => void;

  // Command stats
  commandStats: CommandStats;
  operationsPerSecond: number;
  updateOpsPerSecond: (ops: number) => void;

  // UI state
  isConnected: boolean;
  setConnected: (connected: boolean) => void;

  // Workshop mode
  workshopMode: boolean;
  setWorkshopMode: (mode: boolean) => void;
  currentScenario?: string;
  setCurrentScenario: (scenario: string) => void;
}

export const useAppStore = create<AppState>((set, get) => ({
  // Cache events
  events: [],
  addEvent: (event) => {
    const newEvent: CacheEvent = {
      ...event,
      timestamp: Date.now(),
      command: event.command || 'OTHER'
    };

    set((state) => ({
      events: [...state.events.slice(-100), newEvent] // Keep last 100 events
    }));
  },

  // Hit ratio stats
  hitRatio: { hits: 0, misses: 0, ratio: 0 },
  updateHitRatio: (stats) => {
    set({ hitRatio: stats });
  },

  // Command stats
  commandStats: { GET: 0, SET: 0, DEL: 0, INCR: 0, EXPIRE: 0, OTHER: 0 },
  operationsPerSecond: 0,
  updateOpsPerSecond: (ops) => {
    set({ operationsPerSecond: ops });
  },

  // UI state
  isConnected: false,
  setConnected: (connected) => {
    set({ isConnected: connected });
  }
}));
