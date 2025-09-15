'use client';

import { useEffect, useState } from 'react';
import dynamic from 'next/dynamic';
import { TopBar } from '@/components/TopBar';
import { Legend } from '@/components/Legend';
import { socketManager } from '@/lib/socket';
import { useAppStore } from '@/lib/state';
import { SERVER_URL } from '@/lib/config';

// Dynamically import the Scene component to avoid SSR issues with Three.js
const Scene = dynamic(() => import('@/components/Scene').then(mod => ({ default: mod.Scene })), {
  ssr: false,
  loading: () => <div className="flex items-center justify-center h-screen">Loading 3D scene...</div>
});

export default function HomePage() {
  const { updateHitRatio } = useAppStore();
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
    
    // Connect to Socket.IO
    socketManager.connect();

    // Poll hit ratio stats
    const pollHitRatio = async () => {
      try {
        const response = await fetch(`${SERVER_URL}/hitratio`);
        if (response.ok) {
          const stats = await response.json();
          updateHitRatio(stats);
        }
      } catch (error) {
        console.error('Failed to fetch hit ratio:', error);
      }
    };

    // Initial poll
    pollHitRatio();

    // Set up polling interval
    const interval = setInterval(pollHitRatio, 2000);

    return () => {
      clearInterval(interval);
      socketManager.disconnect();
    };
  }, [updateHitRatio]);

  if (!mounted) {
    return (
      <div className="flex items-center justify-center h-screen">
        <div className="text-center">
          <h1 className="text-2xl font-bold mb-4">Redis Cache Heatmap</h1>
          <p>Loading...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="relative w-full h-screen overflow-hidden">
      {/* 3D Scene */}
      <div className="scene-container">
        <Scene />
      </div>
      
      {/* UI Overlay */}
      <div className="ui-overlay">
        <TopBar />
        <Legend />
      </div>
    </div>
  );
}
