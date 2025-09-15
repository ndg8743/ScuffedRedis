'use client';

import { useEffect, useState } from 'react';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { useAppStore } from '@/lib/state';
import { SERVER_URL } from '@/lib/config';

export function TopBar() {
  const { hitRatio, isConnected } = useAppStore();
  const [isWarmingUp, setIsWarmingUp] = useState(false);

  const handleWarmUp = async () => {
    setIsWarmingUp(true);
    try {
      const response = await fetch(`${SERVER_URL}/warmup?count=50`, {
        method: 'POST',
      });
      if (response.ok) {
        console.log('Cache warmed up successfully');
      }
    } catch (error) {
      console.error('Failed to warm up cache:', error);
    } finally {
      setIsWarmingUp(false);
    }
  };

  const getHitRatioBadgeVariant = () => {
    if (hitRatio.ratio >= 0.8) return 'success';
    if (hitRatio.ratio >= 0.5) return 'warning';
    return 'error';
  };

  const getHitRatioText = () => {
    return `${(hitRatio.ratio * 100).toFixed(1)}%`;
  };

  return (
    <div className="flex items-center justify-between p-4 bg-background/80 backdrop-blur-sm border-b">
      <div className="flex items-center space-x-4">
        <h1 className="text-2xl font-bold">Redis Cache Heatmap</h1>
        <Badge variant={isConnected ? 'success' : 'error'}>
          {isConnected ? 'Connected' : 'Disconnected'}
        </Badge>
      </div>
      
      <div className="flex items-center space-x-4">
        <div className="flex items-center space-x-2">
          <span className="text-sm text-muted-foreground">Hit Ratio:</span>
          <Badge variant={getHitRatioBadgeVariant()}>
            {getHitRatioText()}
          </Badge>
        </div>
        
        <Button 
          onClick={handleWarmUp} 
          disabled={isWarmingUp}
          variant="outline"
        >
          {isWarmingUp ? 'Warming Up...' : 'Warm Up'}
        </Button>
      </div>
    </div>
  );
}
