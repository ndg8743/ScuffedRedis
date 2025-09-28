'use client';

import { Badge } from '@/components/ui/badge';
import { Card, CardContent } from '@/components/ui/card';
import { useAppStore } from '@/lib/state';

export function Legend() {
  const { hitRatio } = useAppStore();

  return (
    <Card className="absolute top-20 left-4 z-20 bg-background/90 backdrop-blur-sm">
      <CardContent className="p-4">
        <div className="space-y-3">
          <h3 className="font-semibold text-sm">Legend</h3>
          
          <div className="space-y-2">
            <div className="flex items-center space-x-2">
              <div className="w-3 h-3 rounded bg-green-500"></div>
              <span className="text-xs">Hit</span>
            </div>
            
            <div className="flex items-center space-x-2">
              <div className="w-3 h-3 rounded bg-red-500"></div>
              <span className="text-xs">Miss</span>
            </div>
            
            <div className="flex items-center space-x-2">
              <div className="w-3 h-3 rounded bg-gray-500"></div>
              <span className="text-xs">Idle</span>
            </div>
          </div>
          
          <div className="pt-2 border-t">
            <div className="text-xs text-muted-foreground">
              Live Hit Ratio: <span className="font-semibold">{(hitRatio.ratio * 100).toFixed(1)}%</span>
            </div>
            <div className="text-xs text-muted-foreground">
              Total: {hitRatio.hits + hitRatio.misses} requests
            </div>
          </div>
        </div>
      </CardContent>
    </Card>
  );
}





