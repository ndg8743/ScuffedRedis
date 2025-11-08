import { io, Socket } from 'socket.io-client';
import { SERVER_URL } from './config';
import { useAppStore } from './state';

class SocketManager {
  private socket: Socket | null = null;
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;

  connect() {
    if (this.socket?.connected) return;

    this.socket = io(SERVER_URL, {
      transports: ['websocket'],
      timeout: 5000,
    });

    this.socket.on('connect', () => {
      console.log('✅ Connected to server');
      this.reconnectAttempts = 0;
      useAppStore.getState().setConnected(true);
    });

    this.socket.on('disconnect', () => {
      console.log('❌ Disconnected from server');
      useAppStore.getState().setConnected(false);
    });

    this.socket.on('connect_error', (error) => {
      console.error('Connection error:', error);
      this.handleReconnect();
    });

    this.socket.on('cache_event', (data) => {
      useAppStore.getState().addEvent({
        id: data.id,
        hit: data.hit,
        latency_ms: data.latency_ms,
        command: data.operationType || 'OTHER'
      });
    });

    this.socket.on('stats_reset', (stats) => {
      useAppStore.getState().updateHitRatio(stats);
      console.log('Statistics reset:', stats);
    });

    this.socket.on('state_loaded', (data) => {
      console.log('Workshop state loaded:', data.scenarioName);
    });

    this.socket.on('initial_stats', (stats) => {
      useAppStore.getState().updateHitRatio(stats);
    });

    return this.socket;
  }

  private handleReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      const delay = Math.min(1000 * Math.pow(2, this.reconnectAttempts), 10000);
      
      console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts})`);
      
      setTimeout(() => {
        this.socket?.connect();
      }, delay);
    } else {
      console.error('Max reconnection attempts reached');
    }
  }

  disconnect() {
    if (this.socket) {
      this.socket.disconnect();
      this.socket = null;
    }
  }

  getSocket() {
    return this.socket;
  }
}

export const socketManager = new SocketManager();
