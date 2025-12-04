'use client';

import { useEffect, useState } from 'react';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { useAppStore } from '@/lib/state';
import { SERVER_URL } from '@/lib/config';
import { Terminal, Zap, BookOpen, Play, Settings } from 'lucide-react';

interface TopBarProps {
  onOpenCommandPlayground: () => void;
  onOpenEducational?: () => void;
  onOpenWorkshop?: () => void;
  onOpenControl?: () => void;
}

export function TopBar({
  onOpenCommandPlayground,
  onOpenEducational,
  onOpenWorkshop,
  onOpenControl
}: TopBarProps) {
  const { hitRatio, isConnected, operationsPerSecond } = useAppStore();
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

        <div className="flex items-center space-x-2">
          <Zap className="w-4 h-4 text-yellow-500 animate-pulse" />
          <span className="text-sm font-medium">{operationsPerSecond.toFixed(1)} ops/sec</span>
        </div>

        <Button
          onClick={onOpenCommandPlayground}
          variant="outline"
          size="sm"
          className="gap-2"
        >
          <Terminal className="w-4 h-4" />
          Command
        </Button>

        {onOpenEducational && (
          <Button
            onClick={onOpenEducational}
            variant="outline"
            size="sm"
            className="gap-2"
          >
            <BookOpen className="w-4 h-4" />
            Learn
          </Button>
        )}

        {onOpenWorkshop && (
          <Button
            onClick={onOpenWorkshop}
            variant="outline"
            size="sm"
            className="gap-2"
          >
            <Play className="w-4 h-4" />
            Scenarios
          </Button>
        )}

        {onOpenControl && (
          <Button
            onClick={onOpenControl}
            variant="outline"
            size="sm"
            className="gap-2"
          >
            <Settings className="w-4 h-4" />
            Control
          </Button>
        )}

        <Button
          onClick={handleWarmUp}
          disabled={isWarmingUp}
          variant="outline"
          size="sm"
        >
          {isWarmingUp ? 'Warming...' : 'Warm Up'}
        </Button>
      </div>
    </div>
  );
}





