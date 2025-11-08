'use client';

import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import {
  Sliders,
  Play,
  Pause,
  RotateCcw,
  Clock,
  TrendingUp,
  Zap,
  Save,
  Upload,
  AlertCircle
} from 'lucide-react';
import { SERVER_URL } from '@/lib/config';
import { useAppStore } from '@/lib/state';

interface WorkshopControlPanelProps {
  isOpen: boolean;
  onClose: () => void;
}

interface TrafficConfig {
  rate: number;
  pattern: 'constant' | 'spike' | 'wave' | 'random';
  operationType: 'read' | 'write' | 'mixed';
}

interface WorkshopState {
  timestamp: number;
  trafficConfig: TrafficConfig;
  hitRatio: any;
  commandStats: any;
  scenarioName?: string;
}

export function WorkshopControlPanel({ isOpen, onClose }: WorkshopControlPanelProps) {
  const { operationsPerSecond, hitRatio } = useAppStore();
  const [trafficConfig, setTrafficConfig] = useState<TrafficConfig>({
    rate: 10,
    pattern: 'constant',
    operationType: 'mixed'
  });
  const [isTrafficRunning, setIsTrafficRunning] = useState(true);
  const [savedStates, setSavedStates] = useState<WorkshopState[]>([]);
  const [isLoading, setIsLoading] = useState(false);

  useEffect(() => {
    // Load saved workshop states
    const saved = localStorage.getItem('workshopStates');
    if (saved) {
      try {
        setSavedStates(JSON.parse(saved));
      } catch (error) {
        console.error('Failed to load saved states:', error);
      }
    }
  }, []);

  const handleTrafficControl = async (action: 'start' | 'stop' | 'pause') => {
    setIsLoading(true);
    try {
      const response = await fetch(`${SERVER_URL}/workshop/traffic`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          action,
          config: action === 'start' ? trafficConfig : undefined
        })
      });

      if (response.ok) {
        setIsTrafficRunning(action !== 'stop');
        console.log(`Traffic ${action} successful`);
      } else {
        console.error(`Failed to ${action} traffic`);
      }
    } catch (error) {
      console.error('Traffic control error:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const handleResetStats = async () => {
    setIsLoading(true);
    try {
      const response = await fetch(`${SERVER_URL}/workshop/reset-stats`, {
        method: 'POST'
      });

      if (response.ok) {
        console.log('Statistics reset successfully');
      }
    } catch (error) {
      console.error('Failed to reset stats:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const handleSaveState = async () => {
    const state: WorkshopState = {
      timestamp: Date.now(),
      trafficConfig,
      hitRatio,
      commandStats: useAppStore.getState().commandStats,
      scenarioName: `Checkpoint ${savedStates.length + 1}`
    };

    try {
      const response = await fetch(`${SERVER_URL}/workshop/save-state`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(state)
      });

      if (response.ok) {
        const newStates = [...savedStates, state];
        setSavedStates(newStates);
        localStorage.setItem('workshopStates', JSON.stringify(newStates));
        console.log('Workshop state saved');
      }
    } catch (error) {
      console.error('Failed to save state:', error);
    }
  };

  const handleLoadState = async (state: WorkshopState) => {
    setIsLoading(true);
    try {
      const response = await fetch(`${SERVER_URL}/workshop/load-state`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(state)
      });

      if (response.ok) {
        setTrafficConfig(state.trafficConfig);
        console.log('Workshop state restored');
      }
    } catch (error) {
      console.error('Failed to load state:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const handleRateChange = (newRate: number) => {
    setTrafficConfig(prev => ({ ...prev, rate: newRate }));
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50">
      <Card className="w-full max-w-2xl max-h-96 overflow-y-auto bg-background">
        <CardHeader className="flex flex-row items-center justify-between space-y-0">
          <div>
            <CardTitle className="flex items-center gap-2">
              <Sliders className="w-5 h-5" />
              Workshop Control Panel
            </CardTitle>
            <CardDescription>
              Manage traffic patterns and workshop state for the 1-hour presentation
            </CardDescription>
          </div>
          <Button variant="ghost" onClick={onClose}>
            Close
          </Button>
        </CardHeader>

        <CardContent className="space-y-6">
          {/* Traffic Control Section */}
          <div className="space-y-4">
            <h3 className="font-semibold flex items-center gap-2">
              <Zap className="w-4 h-4" />
              Traffic Control
            </h3>

            <div className="grid grid-cols-2 gap-4 bg-muted p-4 rounded-lg">
              <div>
                <label className="text-sm font-medium block mb-2">
                  Operations/Second: {trafficConfig.rate}
                </label>
                <input
                  type="range"
                  min="1"
                  max="100"
                  value={trafficConfig.rate}
                  onChange={(e) => handleRateChange(parseInt(e.target.value))}
                  className="w-full"
                  disabled={isLoading}
                />
              </div>

              <div>
                <label className="text-sm font-medium block mb-2">Pattern</label>
                <select
                  value={trafficConfig.pattern}
                  onChange={(e) => setTrafficConfig(prev => ({
                    ...prev,
                    pattern: e.target.value as TrafficConfig['pattern']
                  }))}
                  className="w-full rounded border bg-background px-2 py-1"
                  disabled={isLoading}
                >
                  <option value="constant">Constant</option>
                  <option value="spike">Spike</option>
                  <option value="wave">Wave</option>
                  <option value="random">Random</option>
                </select>
              </div>

              <div>
                <label className="text-sm font-medium block mb-2">Operation Type</label>
                <select
                  value={trafficConfig.operationType}
                  onChange={(e) => setTrafficConfig(prev => ({
                    ...prev,
                    operationType: e.target.value as TrafficConfig['operationType']
                  }))}
                  className="w-full rounded border bg-background px-2 py-1"
                  disabled={isLoading}
                >
                  <option value="read">Read Heavy</option>
                  <option value="write">Write Heavy</option>
                  <option value="mixed">Mixed</option>
                </select>
              </div>

              <div className="flex items-end gap-2">
                <Badge variant={isTrafficRunning ? 'success' : 'secondary'}>
                  {isTrafficRunning ? 'Running' : 'Paused'}
                </Badge>
              </div>
            </div>

            <div className="flex gap-2">
              <Button
                onClick={() => handleTrafficControl('start')}
                disabled={isTrafficRunning || isLoading}
                className="gap-2"
              >
                <Play className="w-4 h-4" />
                Start
              </Button>
              <Button
                onClick={() => handleTrafficControl('pause')}
                disabled={!isTrafficRunning || isLoading}
                variant="outline"
                className="gap-2"
              >
                <Pause className="w-4 h-4" />
                Pause
              </Button>
              <Button
                onClick={() => handleTrafficControl('stop')}
                disabled={isLoading}
                variant="destructive"
                className="gap-2"
              >
                <RotateCcw className="w-4 h-4" />
                Stop
              </Button>
            </div>
          </div>

          <Separator />

          {/* Statistics Section */}
          <div className="space-y-4">
            <h3 className="font-semibold flex items-center gap-2">
              <TrendingUp className="w-4 h-4" />
              Performance Metrics
            </h3>

            <div className="grid grid-cols-2 gap-4 text-sm">
              <div className="bg-muted p-3 rounded">
                <p className="text-muted-foreground">Current Ops/Sec</p>
                <p className="text-lg font-semibold">{operationsPerSecond}</p>
              </div>
              <div className="bg-muted p-3 rounded">
                <p className="text-muted-foreground">Hit Ratio</p>
                <p className="text-lg font-semibold">{(hitRatio.ratio * 100).toFixed(1)}%</p>
              </div>
            </div>

            <Button
              onClick={handleResetStats}
              disabled={isLoading}
              variant="outline"
              className="w-full"
            >
              <RotateCcw className="w-4 h-4 mr-2" />
              Reset Statistics
            </Button>
          </div>

          <Separator />

          {/* Checkpoint Management */}
          <div className="space-y-4">
            <h3 className="font-semibold flex items-center gap-2">
              <Save className="w-4 h-4" />
              Checkpoints ({savedStates.length})
            </h3>

            <Button
              onClick={handleSaveState}
              disabled={isLoading}
              className="w-full"
            >
              <Save className="w-4 h-4 mr-2" />
              Save Current State as Checkpoint
            </Button>

            {savedStates.length > 0 && (
              <div className="space-y-2 max-h-40 overflow-y-auto">
                {savedStates.map((state, index) => (
                  <div
                    key={index}
                    className="flex items-center justify-between bg-muted p-3 rounded text-sm"
                  >
                    <div>
                      <p className="font-medium">{state.scenarioName}</p>
                      <p className="text-xs text-muted-foreground">
                        {new Date(state.timestamp).toLocaleTimeString()}
                      </p>
                    </div>
                    <Button
                      onClick={() => handleLoadState(state)}
                      disabled={isLoading}
                      variant="outline"
                      size="sm"
                    >
                      <Upload className="w-3 h-3 mr-1" />
                      Load
                    </Button>
                  </div>
                ))}
              </div>
            )}
          </div>
        </CardContent>
      </Card>
    </div>
  );
}
