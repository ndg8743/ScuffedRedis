'use client';

import React, { useState, useEffect, useRef } from 'react';
import { Card, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Separator } from '@/components/ui/separator';
import {
  Play,
  Pause,
  Square,
  ChevronRight,
  Clock,
  BarChart3,
  Zap,
  Database,
  Users,
  Trash2,
  Download,
  Upload,
  Sliders,
  AlertCircle,
  CheckCircle,
  GripVertical,
  ChevronDown,
  ChevronUp
} from 'lucide-react';
import { SERVER_URL } from '@/lib/config';
import { useAppStore } from '@/lib/state';

// State types for presentation, traffic, and metrics
interface PresentationState {
  isRunning: boolean;
  isPaused: boolean;
  currentScenario: string;
  currentStep: number;
  elapsedSeconds: number;
}

interface TrafficConfig {
  rate: number;
  pattern: 'uniform' | 'zipf' | 'burst';
  isPaused: boolean;
}

interface MetricsSnapshot {
  opsPerSecond: number;
  memoryUsageMB: number;
  activeConnections: number;
  commandDistribution: Record<string, number>;
  timestamp: number;
}

interface WorkshopState {
  presentationState: PresentationState;
  trafficConfig: TrafficConfig;
  metrics: MetricsSnapshot[];
}

export function WorkshopControlPanel() {
  // Presentation mode state
  const [presentationState, setPresentationState] = useState<PresentationState>({
    isRunning: false,
    isPaused: false,
    currentScenario: 'none',
    currentStep: 0,
    elapsedSeconds: 0
  });

  // Traffic control state
  const [trafficConfig, setTrafficConfig] = useState<TrafficConfig>({
    rate: 10,
    pattern: 'uniform',
    isPaused: false
  });

  // Metrics history
  const [metrics, setMetrics] = useState<MetricsSnapshot[]>([]);

  // UI collapse state
  const [expandedSections, setExpandedSections] = useState<Record<string, boolean>>({
    presentation: true,
    traffic: true,
    actions: false,
    metrics: true,
    scenarios: false
  });

  // Draggable panel positioning
  const [position, setPosition] = useState({ x: 20, y: 20 });
  const [isDragging, setIsDragging] = useState(false);
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });
  const panelRef = useRef<HTMLDivElement>(null);

  const { hitRatio } = useAppStore();

  // Timer for presentation duration
  useEffect(() => {
    if (!presentationState.isRunning || presentationState.isPaused) return;

    const timer = setInterval(() => {
      setPresentationState(prev => ({
        ...prev,
        elapsedSeconds: prev.elapsedSeconds + 1
      }));
    }, 1000);

    return () => clearInterval(timer);
  }, [presentationState.isRunning, presentationState.isPaused]);

  // Poll metrics from server every second
  useEffect(() => {
    const pollMetrics = async () => {
      try {
        const response = await fetch(`${SERVER_URL}/metrics`, {
          method: 'GET'
        });
        if (response.ok) {
          const data = await response.json();
          setMetrics(prev => [
            ...prev.slice(-59),
            {
              opsPerSecond: data.opsPerSecond || 0,
              memoryUsageMB: data.memoryUsageMB || 0,
              activeConnections: data.activeConnections || 0,
              commandDistribution: data.commandDistribution || {},
              timestamp: Date.now()
            }
          ]);
        }
      } catch (error) {
        console.error('Failed to fetch metrics:', error);
      }
    };

    const interval = setInterval(pollMetrics, 1000);
    return () => clearInterval(interval);
  }, []);

  // Format seconds into MM:SS display
  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  };

  // Get most recent metrics
  const latestMetric = metrics[metrics.length - 1] || {
    opsPerSecond: 0,
    memoryUsageMB: 0,
    activeConnections: 0,
    commandDistribution: {}
  };

  // Dragging handlers for floating panel
  const handleMouseDown = (e: React.MouseEvent<HTMLDivElement>) => {
    if ((e.target as HTMLElement).closest('[data-drag-handle]')) {
      setIsDragging(true);
      setDragOffset({
        x: e.clientX - position.x,
        y: e.clientY - position.y
      });
    }
  };

  const handleMouseMove = (e: React.MouseEvent<HTMLDivElement>) => {
    if (!isDragging) return;
    setPosition({
      x: e.clientX - dragOffset.x,
      y: e.clientY - dragOffset.y
    });
  };

  const handleMouseUp = () => {
    setIsDragging(false);
  };

  // Presentation control handlers
  const handleStartPresentation = async () => {
    setPresentationState(prev => ({
      ...prev,
      isRunning: true,
      isPaused: false,
      elapsedSeconds: 0
    }));
  };

  const handlePausePresentation = () => {
    setPresentationState(prev => ({
      ...prev,
      isPaused: !prev.isPaused
    }));
  };

  const handleStopPresentation = () => {
    setPresentationState({
      isRunning: false,
      isPaused: false,
      currentScenario: 'none',
      currentStep: 0,
      elapsedSeconds: 0
    });
  };

  const handleNextStep = () => {
    setPresentationState(prev => ({
      ...prev,
      currentStep: prev.currentStep + 1
    }));
  };

  // Traffic control handlers
  const handleTrafficRateChange = async (newRate: number) => {
    setTrafficConfig(prev => ({
      ...prev,
      rate: newRate
    }));

    try {
      await fetch(`${SERVER_URL}/traffic/rate`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ rate: newRate })
      });
    } catch (error) {
      console.error('Failed to update traffic rate:', error);
    }
  };

  const handleTrafficPatternChange = async (pattern: 'uniform' | 'zipf' | 'burst') => {
    setTrafficConfig(prev => ({
      ...prev,
      pattern
    }));

    try {
      await fetch(`${SERVER_URL}/traffic/pattern`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ pattern })
      });
    } catch (error) {
      console.error('Failed to update traffic pattern:', error);
    }
  };

  const handleToggleTraffic = async () => {
    const newState = !trafficConfig.isPaused;
    setTrafficConfig(prev => ({
      ...prev,
      isPaused: newState
    }));

    try {
      await fetch(`${SERVER_URL}/traffic/${newState ? 'pause' : 'resume'}`, {
        method: 'POST'
      });
    } catch (error) {
      console.error('Failed to toggle traffic:', error);
    }
  };

  // Quick action handlers
  const handlePopulateTestData = async () => {
    try {
      const response = await fetch(`${SERVER_URL}/test-data`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ count: 100 })
      });
      if (response.ok) {
        console.log('Test data populated');
      }
    } catch (error) {
      console.error('Failed to populate test data:', error);
    }
  };

  const handleClearCache = async () => {
    try {
      const response = await fetch(`${SERVER_URL}/clear`, {
        method: 'POST'
      });
      if (response.ok) {
        console.log('Cache cleared');
      }
    } catch (error) {
      console.error('Failed to clear cache:', error);
    }
  };

  const handleResetStats = async () => {
    try {
      const response = await fetch(`${SERVER_URL}/reset-stats`, {
        method: 'POST'
      });
      if (response.ok) {
        console.log('Stats reset');
      }
    } catch (error) {
      console.error('Failed to reset stats:', error);
    }
  };

  // State persistence handlers
  const handleSaveState = () => {
    const state: WorkshopState = {
      presentationState,
      trafficConfig,
      metrics
    };

    const dataStr = JSON.stringify(state, null, 2);
    const dataBlob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(dataBlob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `workshop-state-${Date.now()}.json`;
    link.click();
    URL.revokeObjectURL(url);
  };

  const handleLoadState = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (!file) return;

      const reader = new FileReader();
      reader.onload = (event) => {
        try {
          const state: WorkshopState = JSON.parse(event.target?.result as string);
          setPresentationState(state.presentationState);
          setTrafficConfig(state.trafficConfig);
          setMetrics(state.metrics);
        } catch (error) {
          console.error('Failed to load state:', error);
        }
      };
      reader.readAsText(file);
    };
    input.click();
  };

  // Toggle section expansion
  const toggleSection = (section: string) => {
    setExpandedSections(prev => ({
      ...prev,
      [section]: !prev[section]
    }));
  };

  // Start a scenario
  const startScenario = async (scenarioId: string) => {
    setPresentationState(prev => ({
      ...prev,
      currentScenario: scenarioId,
      currentStep: 0,
      isRunning: true,
      isPaused: false,
      elapsedSeconds: 0
    }));
  };

  // Get top 5 commands from distribution
  const getCommandDistribution = () => {
    const distribution = latestMetric.commandDistribution || {};
    return Object.entries(distribution)
      .sort(([, a], [, b]) => b - a)
      .slice(0, 5);
  };

  return (
    <div
      ref={panelRef}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
      style={{
        position: 'fixed',
        left: `${position.x}px`,
        top: `${position.y}px`,
        zIndex: 50,
        userSelect: isDragging ? 'none' : 'auto',
        cursor: isDragging ? 'grabbing' : 'auto'
      }}
      className="w-96 max-h-[90vh] overflow-y-auto"
    >
      <Card className="shadow-lg border-2 border-primary/30">
        {/* Draggable header with grip handle */}
        <div
          data-drag-handle
          onMouseDown={handleMouseDown}
          className="flex items-center justify-between p-4 bg-gradient-to-r from-primary/10 to-primary/5 border-b cursor-grab active:cursor-grabbing"
        >
          <div className="flex items-center gap-2">
            <GripVertical className="w-4 h-4 text-muted-foreground" />
            <h2 className="font-bold text-lg">Workshop Control</h2>
          </div>
          <Badge variant="outline">Live</Badge>
        </div>

        <CardContent className="p-0">
          {/* Presentation Mode Controls Section */}
          <div className="border-b">
            <button
              onClick={() => toggleSection('presentation')}
              className="w-full flex items-center justify-between p-4 hover:bg-accent/50"
            >
              <div className="flex items-center gap-2">
                <Play className="w-4 h-4" />
                <span className="font-semibold">Presentation Mode</span>
              </div>
              {expandedSections.presentation ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {expandedSections.presentation && (
              <div className="px-4 pb-4 space-y-4">
                {/* Start, pause, stop buttons */}
                <div className="flex gap-2">
                  <Button
                    onClick={handleStartPresentation}
                    disabled={presentationState.isRunning}
                    className="flex-1 gap-2"
                    size="sm"
                  >
                    <Play className="w-4 h-4" />
                    Start
                  </Button>
                  <Button
                    onClick={handlePausePresentation}
                    disabled={!presentationState.isRunning}
                    variant="outline"
                    className="flex-1 gap-2"
                    size="sm"
                  >
                    <Pause className="w-4 h-4" />
                    {presentationState.isPaused ? 'Resume' : 'Pause'}
                  </Button>
                  <Button
                    onClick={handleStopPresentation}
                    disabled={!presentationState.isRunning}
                    variant="destructive"
                    className="flex-1 gap-2"
                    size="sm"
                  >
                    <Square className="w-4 h-4" />
                    Stop
                  </Button>
                </div>

                {/* Timer display */}
                <div className="bg-secondary/20 p-3 rounded-md flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <Clock className="w-4 h-4 text-muted-foreground" />
                    <span className="text-sm text-muted-foreground">Duration:</span>
                  </div>
                  <span className="font-mono font-bold text-lg">
                    {formatTime(presentationState.elapsedSeconds)}
                  </span>
                </div>

                {/* Current scenario info */}
                {presentationState.isRunning && (
                  <div className="bg-primary/5 p-3 rounded-md">
                    <p className="text-xs text-muted-foreground mb-1">Current Scenario:</p>
                    <p className="font-semibold capitalize">
                      {presentationState.currentScenario === 'none' ? 'None selected' : presentationState.currentScenario}
                    </p>
                    <p className="text-xs text-muted-foreground mt-1">
                      Step {presentationState.currentStep}
                    </p>
                  </div>
                )}

                {/* Next step button */}
                <Button
                  onClick={handleNextStep}
                  disabled={!presentationState.isRunning || presentationState.currentScenario === 'none'}
                  variant="outline"
                  className="w-full gap-2"
                  size="sm"
                >
                  <ChevronRight className="w-4 h-4" />
                  Next Step
                </Button>
              </div>
            )}
          </div>

          {/* Traffic Control Section */}
          <div className="border-b">
            <button
              onClick={() => toggleSection('traffic')}
              className="w-full flex items-center justify-between p-4 hover:bg-accent/50"
            >
              <div className="flex items-center gap-2">
                <Zap className="w-4 h-4" />
                <span className="font-semibold">Traffic Control</span>
              </div>
              {expandedSections.traffic ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {expandedSections.traffic && (
              <div className="px-4 pb-4 space-y-4">
                {/* Rate slider */}
                <div>
                  <div className="flex items-center justify-between mb-2">
                    <label className="text-sm font-medium">Rate (ops/sec)</label>
                    <Badge variant="secondary">{trafficConfig.rate}</Badge>
                  </div>
                  <input
                    type="range"
                    min="1"
                    max="100"
                    value={trafficConfig.rate}
                    onChange={(e) => handleTrafficRateChange(Number(e.target.value))}
                    className="w-full"
                  />
                </div>

                {/* Pattern selector */}
                <div>
                  <label className="text-sm font-medium block mb-2">Pattern</label>
                  <div className="grid grid-cols-3 gap-2">
                    {['uniform', 'zipf', 'burst'].map(pattern => (
                      <Button
                        key={pattern}
                        onClick={() => handleTrafficPatternChange(pattern as any)}
                        variant={trafficConfig.pattern === pattern ? 'default' : 'outline'}
                        size="sm"
                        className="capitalize"
                      >
                        {pattern}
                      </Button>
                    ))}
                  </div>
                </div>

                {/* Pause/resume traffic button */}
                <Button
                  onClick={handleToggleTraffic}
                  variant={trafficConfig.isPaused ? 'outline' : 'default'}
                  className="w-full gap-2"
                  size="sm"
                >
                  <Zap className="w-4 h-4" />
                  {trafficConfig.isPaused ? 'Resume Traffic' : 'Pause Traffic'}
                </Button>
              </div>
            )}
          </div>

          {/* Quick Actions Section */}
          <div className="border-b">
            <button
              onClick={() => toggleSection('actions')}
              className="w-full flex items-center justify-between p-4 hover:bg-accent/50"
            >
              <div className="flex items-center gap-2">
                <Sliders className="w-4 h-4" />
                <span className="font-semibold">Quick Actions</span>
              </div>
              {expandedSections.actions ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {expandedSections.actions && (
              <div className="px-4 pb-4 space-y-2">
                <Button
                  onClick={handlePopulateTestData}
                  variant="outline"
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <Database className="w-4 h-4" />
                  Populate Test Data
                </Button>
                <Button
                  onClick={handleClearCache}
                  variant="outline"
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <Trash2 className="w-4 h-4" />
                  Clear Cache
                </Button>
                <Button
                  onClick={handleResetStats}
                  variant="outline"
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <AlertCircle className="w-4 h-4" />
                  Reset Statistics
                </Button>
                <Separator className="my-2" />
                <Button
                  onClick={handleSaveState}
                  variant="outline"
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <Download className="w-4 h-4" />
                  Save State
                </Button>
                <Button
                  onClick={handleLoadState}
                  variant="outline"
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <Upload className="w-4 h-4" />
                  Load State
                </Button>
              </div>
            )}
          </div>

          {/* Live Metrics Dashboard Section */}
          <div className="border-b">
            <button
              onClick={() => toggleSection('metrics')}
              className="w-full flex items-center justify-between p-4 hover:bg-accent/50"
            >
              <div className="flex items-center gap-2">
                <BarChart3 className="w-4 h-4" />
                <span className="font-semibold">Live Metrics</span>
              </div>
              {expandedSections.metrics ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {expandedSections.metrics && (
              <div className="px-4 pb-4 space-y-3">
                {/* Operations per second */}
                <div className="bg-secondary/20 p-3 rounded-md">
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Ops/sec:</span>
                    <span className="font-bold text-lg">
                      {latestMetric.opsPerSecond.toFixed(1)}
                    </span>
                  </div>
                  <div className="text-xs text-muted-foreground mt-1">
                    Last {metrics.length}s of data
                  </div>
                </div>

                {/* Memory usage */}
                <div className="bg-secondary/20 p-3 rounded-md">
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Memory:</span>
                    <span className="font-bold text-lg">
                      {latestMetric.memoryUsageMB.toFixed(1)} MB
                    </span>
                  </div>
                </div>

                {/* Active connections */}
                <div className="bg-secondary/20 p-3 rounded-md">
                  <div className="flex items-center justify-between">
                    <div className="flex items-center gap-2">
                      <Users className="w-4 h-4 text-muted-foreground" />
                      <span className="text-sm text-muted-foreground">Connections:</span>
                    </div>
                    <span className="font-bold text-lg">
                      {latestMetric.activeConnections}
                    </span>
                  </div>
                </div>

                {/* Hit ratio */}
                <div className="bg-secondary/20 p-3 rounded-md">
                  <div className="flex items-center justify-between">
                    <span className="text-sm text-muted-foreground">Hit Ratio:</span>
                    <span className="font-bold text-lg">
                      {(hitRatio.ratio * 100).toFixed(1)}%
                    </span>
                  </div>
                </div>

                {/* Top commands distribution */}
                {getCommandDistribution().length > 0 && (
                  <div className="bg-secondary/20 p-3 rounded-md">
                    <p className="text-sm font-medium mb-2">Top Commands:</p>
                    <div className="space-y-1">
                      {getCommandDistribution().map(([cmd, count]) => (
                        <div key={cmd} className="flex items-center justify-between text-xs">
                          <span className="font-mono">{cmd}</span>
                          <Badge variant="secondary">{count}</Badge>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            )}
          </div>

          {/* Scenario Quick Launch Section */}
          <div>
            <button
              onClick={() => toggleSection('scenarios')}
              className="w-full flex items-center justify-between p-4 hover:bg-accent/50"
            >
              <div className="flex items-center gap-2">
                <CheckCircle className="w-4 h-4" />
                <span className="font-semibold">Quick Launch</span>
              </div>
              {expandedSections.scenarios ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {expandedSections.scenarios && (
              <div className="px-4 pb-4 space-y-2">
                <Button
                  onClick={() => startScenario('cold-vs-warm')}
                  variant={presentationState.currentScenario === 'cold-vs-warm' ? 'default' : 'outline'}
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <span className="text-xs font-medium">Cold vs Warm</span>
                  <Badge variant="secondary" className="ml-auto text-xs">Beginner</Badge>
                </Button>
                <Button
                  onClick={() => startScenario('ttl-expiration')}
                  variant={presentationState.currentScenario === 'ttl-expiration' ? 'default' : 'outline'}
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <span className="text-xs font-medium">TTL & Expiration</span>
                  <Badge variant="secondary" className="ml-auto text-xs">Intermediate</Badge>
                </Button>
                <Button
                  onClick={() => startScenario('data-types')}
                  variant={presentationState.currentScenario === 'data-types' ? 'default' : 'outline'}
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <span className="text-xs font-medium">Data Types</span>
                  <Badge variant="secondary" className="ml-auto text-xs">Intermediate</Badge>
                </Button>
                <Button
                  onClick={() => startScenario('eviction')}
                  variant={presentationState.currentScenario === 'eviction' ? 'default' : 'outline'}
                  className="w-full justify-start gap-2"
                  size="sm"
                >
                  <span className="text-xs font-medium">Eviction Policies</span>
                  <Badge variant="secondary" className="ml-auto text-xs">Advanced</Badge>
                </Button>
              </div>
            )}
          </div>
        </CardContent>
      </Card>
    </div>
  );
}
