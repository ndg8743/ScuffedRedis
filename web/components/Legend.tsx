'use client';

import { Badge } from '@/components/ui/badge';
import { Card, CardContent } from '@/components/ui/card';
import { useAppStore } from '@/lib/state';

export function Legend() {
  const { hitRatio } = useAppStore();

  // Command type colors matching config
  const commandColors = {
    GET: 'bg-green-500',
    SET: 'bg-blue-500',
    DEL: 'bg-red-500',
    INCR: 'bg-yellow-500',
    EXPIRE: 'bg-orange-500',
    OTHER: 'bg-blue-300'
  };

  return (
    <Card className="absolute top-20 left-4 z-20 bg-background/90 backdrop-blur-sm">
      <CardContent className="p-4">
        <div className="space-y-3">
          <h3 className="font-semibold text-sm">Redis Commands</h3>

          {/* Command type indicators */}
          <div className="space-y-2">
            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.GET}`}></div>
              <span className="text-xs">GET (smooth pulse)</span>
            </div>

            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.SET}`}></div>
              <span className="text-xs">SET (bouncy)</span>
            </div>

            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.DEL}`}></div>
              <span className="text-xs">DEL (sharp spike)</span>
            </div>

            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.INCR}`}></div>
              <span className="text-xs">INCR (wave)</span>
            </div>

            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.EXPIRE}`}></div>
              <span className="text-xs">EXPIRE (fade)</span>
            </div>

            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded ${commandColors.OTHER}`}></div>
              <span className="text-xs">OTHER (smooth)</span>
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
