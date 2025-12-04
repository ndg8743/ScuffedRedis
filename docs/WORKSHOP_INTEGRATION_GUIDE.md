# Workshop Control Panel Integration Guide

Complete guide for integrating the WorkshopControlPanel component into your Redis workshop application.

## Quick Start

### 1. Add Component to Your Page

```tsx
// app/page.tsx or your main layout
'use client';

import { WorkshopControlPanel } from '@/components/WorkshopControlPanel';
import { YourMainVisualizations } from '@/components/YourVisualizations';

export default function Home() {
  return (
    <div className="w-full h-screen bg-background">
      {/* Your main content */}
      <YourMainVisualizations />

      {/* Floating workshop control panel */}
      <WorkshopControlPanel />
    </div>
  );
}
```

The component is stateful and self-contained - no props or additional setup needed.

## Server Implementation

### Required API Endpoints

Implement these endpoints in your backend to enable all features:

#### 1. Traffic Control Endpoints

```typescript
// POST /traffic/rate
// Adjust traffic generation rate
Request:  { rate: number }  // 1-100 ops/sec
Response: { success: true }

// POST /traffic/pattern
// Change traffic distribution pattern
Request:  { pattern: 'uniform' | 'zipf' | 'burst' }
Response: { success: true }

// POST /traffic/pause
// Pause traffic generation
Response: { success: true }

// POST /traffic/resume
// Resume traffic generation
Response: { success: true }
```

#### 2. Quick Actions Endpoints

```typescript
// POST /test-data
// Populate cache with test data
Request:  { count: number }
Response: { success: true, inserted: number }

// POST /clear
// Clear all cache entries
Response: { success: true }

// POST /reset-stats
// Reset performance statistics
Response: { success: true }
```

#### 3. Metrics Endpoint

```typescript
// GET /metrics
// Return current performance metrics
Response: {
  opsPerSecond: number,
  memoryUsageMB: number,
  activeConnections: number,
  commandDistribution: {
    [command: string]: number
  }
}
```

### Example Express.js Implementation

```typescript
import express from 'express';
import { TrafficGenerator } from './traffic';
import { MetricsCollector } from './metrics';

const app = express();
app.use(express.json());

const traffic = new TrafficGenerator();
const metrics = new MetricsCollector();

// Traffic Control
app.post('/traffic/rate', (req, res) => {
  const { rate } = req.body;
  traffic.setRate(rate);
  res.json({ success: true });
});

app.post('/traffic/pattern', (req, res) => {
  const { pattern } = req.body;
  traffic.setPattern(pattern);
  res.json({ success: true });
});

app.post('/traffic/pause', (req, res) => {
  traffic.pause();
  res.json({ success: true });
});

app.post('/traffic/resume', (req, res) => {
  traffic.resume();
  res.json({ success: true });
});

// Quick Actions
app.post('/test-data', (req, res) => {
  const { count } = req.body;
  const inserted = traffic.populateTestData(count);
  res.json({ success: true, inserted });
});

app.post('/clear', (req, res) => {
  traffic.clearCache();
  res.json({ success: true });
});

app.post('/reset-stats', (req, res) => {
  metrics.reset();
  res.json({ success: true });
});

// Metrics
app.get('/metrics', (req, res) => {
  res.json({
    opsPerSecond: metrics.getOpsPerSecond(),
    memoryUsageMB: metrics.getMemoryUsage(),
    activeConnections: metrics.getActiveConnections(),
    commandDistribution: metrics.getCommandDistribution()
  });
});

app.listen(4000, () => console.log('Server running on :4000'));
```

## Zustand Store Integration

The component reads from the AppStore. Ensure your store includes:

```typescript
// lib/state.ts
import { create } from 'zustand';

interface HitRatioStats {
  hits: number;
  misses: number;
  ratio: number;  // 0-1
}

interface AppState {
  hitRatio: HitRatioStats;
  updateHitRatio: (stats: HitRatioStats) => void;
  // ... other state
}

export const useAppStore = create<AppState>((set) => ({
  hitRatio: { hits: 0, misses: 0, ratio: 0 },
  updateHitRatio: (stats) => set({ hitRatio: stats }),
  // ... other methods
}));
```

## State Management Patterns

### Persisting Metrics Across Sessions

The component saves/loads complete state including metrics history. To preserve metrics in your backend:

```typescript
// lib/metricsStore.ts
import { MetricsSnapshot } from '@/components/WorkshopControlPanel';

class MetricsStore {
  private history: MetricsSnapshot[] = [];

  add(snapshot: MetricsSnapshot) {
    this.history.push(snapshot);
    // Keep only last 60 seconds
    if (this.history.length > 60) {
      this.history.shift();
    }
  }

  export() {
    return JSON.stringify(this.history);
  }

  import(json: string) {
    this.history = JSON.parse(json);
  }
}
```

### Scenario Progress Tracking

Store scenario progress in your backend for multi-session workshops:

```typescript
interface ScenarioProgress {
  scenarioId: string;
  currentStep: number;
  completedAt?: number;
  notes?: string;
}

// POST /scenario/:id/progress
app.post('/scenario/:id/progress', (req, res) => {
  const { step, notes } = req.body;
  // Store in database
  res.json({ success: true });
});

// GET /scenario/:id/progress
app.get('/scenario/:id/progress', (req, res) => {
  // Retrieve from database
  res.json({ step: 3, completedAt: null });
});
```

## Customization

### Modifying Scenario List

Edit the scenario buttons in the component:

```tsx
// Around line 750 in WorkshopControlPanel.tsx
<Button
  onClick={() => startScenario('your-scenario')}
  variant={presentationState.currentScenario === 'your-scenario' ? 'default' : 'outline'}
  className="w-full justify-start gap-2"
  size="sm"
>
  <span className="text-xs font-medium">Your Scenario Name</span>
  <Badge variant="secondary" className="ml-auto text-xs">Difficulty</Badge>
</Button>
```

### Adjusting Metrics Polling Rate

Change polling interval (default 1000ms):

```tsx
// Around line 136
const interval = setInterval(pollMetrics, 2000); // 2 seconds instead
```

### Customizing Traffic Rate Range

Modify slider min/max values:

```tsx
// Around line 517
<input
  type="range"
  min="1"
  max="200"  // Changed from 100
  value={trafficConfig.rate}
  // ...
/>
```

## Styling & Theming

The component uses Tailwind CSS and respects your theme variables.

### Dark Mode Support

Works automatically with your theme. Set in `globals.css`:

```css
:root {
  --background: 0 0% 100%;
  --foreground: 0 0% 3.6%;
  --primary: 0 72% 51%;
  --secondary: 240 10% 95%;
  /* ... */
}

@media (prefers-color-scheme: dark) {
  :root {
    --background: 0 0% 3.6%;
    --foreground: 0 0% 98%;
    --primary: 0 72% 51%;
    /* ... */
  }
}
```

### Custom Colors

Override colors in your component wrapper:

```tsx
<div className="[--primary:#ff0000] [--secondary:#0000ff]">
  <WorkshopControlPanel />
</div>
```

## Performance Optimization

### Reducing Metrics Storage

Keep memory usage low by limiting metrics history:

```tsx
setMetrics(prev => [
  ...prev.slice(-30),  // Keep last 30s instead of 60s
  newMetric
]);
```

### Debouncing Traffic Rate Changes

Prevent excessive API calls during slider drag:

```tsx
import { useMemo } from 'react';

// Create debounced handler
const debouncedRateChange = useMemo(
  () => debounce((rate: number) => handleTrafficRateChange(rate), 300),
  []
);

// Use in onChange
onChange={(e) => debouncedRateChange(Number(e.target.value))}
```

### Memoizing Heavy Computations

```tsx
const getCommandDistribution = () => {
  return useMemo(() => {
    const distribution = latestMetric.commandDistribution || {};
    return Object.entries(distribution)
      .sort(([, a], [, b]) => b - a)
      .slice(0, 5);
  }, [latestMetric.commandDistribution]);
};
```

## Testing

### Unit Test Example

```typescript
import { render, screen, fireEvent } from '@testing-library/react';
import { WorkshopControlPanel } from '@/components/WorkshopControlPanel';

describe('WorkshopControlPanel', () => {
  it('should start presentation on Start click', () => {
    render(<WorkshopControlPanel />);
    const startBtn = screen.getByRole('button', { name: /start/i });
    fireEvent.click(startBtn);
    // Assert presentation started
  });

  it('should update traffic rate', () => {
    render(<WorkshopControlPanel />);
    const slider = screen.getByRole('slider');
    fireEvent.change(slider, { target: { value: '50' } });
    // Assert API call made
  });
});
```

### Manual Testing Checklist

- [ ] Dragging panel works smoothly
- [ ] Presentation timer increments correctly
- [ ] Traffic rate slider sends API requests
- [ ] Metrics update every second
- [ ] Scenario quick launch buttons work
- [ ] Save state creates downloadable JSON
- [ ] Load state restores previous environment
- [ ] All sections collapse/expand properly
- [ ] Buttons disable when not applicable
- [ ] Error handling shows in console

## Troubleshooting

### Metrics Not Appearing

Check browser DevTools Network tab:
1. Verify `/metrics` endpoint responds with 200 status
2. Check response has correct JSON structure
3. Verify `SERVER_URL` config points to correct backend

### Dragging Doesn't Work

Ensure dragging is initiated on the header with `data-drag-handle` attribute.

### State Save/Load Fails

File must be valid JSON matching `WorkshopState` interface. Check:
```json
{
  "presentationState": { ... },
  "trafficConfig": { ... },
  "metrics": [ ... ]
}
```

### Sections Don't Collapse

Browser console should show no errors. Verify toggle function is being called.

## Advanced Integration

### Real-Time Collaboration

Broadcast state changes via WebSocket for multi-presenter scenarios:

```typescript
import { io } from 'socket.io-client';

const socket = io(SERVER_URL);

socket.on('traffic-changed', (config) => {
  setTrafficConfig(config);
});

socket.on('presentation-state-changed', (state) => {
  setPresentationState(state);
});
```

### Analytics Integration

Log workshop metrics for post-session analysis:

```typescript
const logMetrics = (state: WorkshopState) => {
  fetch('/api/workshop-analytics', {
    method: 'POST',
    body: JSON.stringify({
      timestamp: Date.now(),
      duration: state.presentationState.elapsedSeconds,
      finalHitRatio: state.metrics[state.metrics.length - 1]?.hitRatio,
      scenariosRun: countUniqueSce narios(state.metrics)
    })
  });
};
```

### Snapshot Comparison

Compare multiple workshop states side-by-side:

```typescript
const compareSnapshots = (state1: WorkshopState, state2: WorkshopState) => {
  return {
    duration: state2.presentationState.elapsedSeconds - state1.presentationState.elapsedSeconds,
    opsPerSecondDiff: latestMetric(state2).opsPerSecond - latestMetric(state1).opsPerSecond,
    hitRatioDiff: calculateHitRatioDiff(state1, state2)
  };
};
```

## Support & Feedback

For issues or feature requests, open an issue in the repository with:
- Current component version
- Browser and OS
- Steps to reproduce
- Expected vs actual behavior
