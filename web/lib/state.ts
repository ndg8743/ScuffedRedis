import { create } from 'zustand';

export interface CacheEvent {
  id: number;
  hit: boolean;
  latency_ms: number;
  timestamp: number;
}

export interface HitRatioStats {
  hits: number;
  misses: number;
  ratio: number;
}

interface AppState {
  // Cache events
  events: CacheEvent[];
  addEvent: (event: Omit<CacheEvent, 'timestamp'>) => void;
  
  // Hit ratio stats
  hitRatio: HitRatioStats;
  updateHitRatio: (stats: HitRatioStats) => void;
  
  // UI state
  isConnected: boolean;
  setConnected: (connected: boolean) => void;
}

export const useAppStore = create<AppState>((set, get) => ({
  // Cache events
  events: [],
  addEvent: (event) => {
    const newEvent: CacheEvent = {
      ...event,
      timestamp: Date.now()
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
  
  // UI state
  isConnected: false,
  setConnected: (connected) => {
    set({ isConnected: connected });
  }
}));





