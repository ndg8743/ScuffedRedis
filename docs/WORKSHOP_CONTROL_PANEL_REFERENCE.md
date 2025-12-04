# Workshop Control Panel - Quick Reference

## Component Location
`/Users/nathan/Downloads/ScuffedRedis/web/components/WorkshopControlPanel.tsx`

## Import
```tsx
import { WorkshopControlPanel } from '@/components/WorkshopControlPanel';
```

## Usage
```tsx
<WorkshopControlPanel />
```

No props required. Component is fully self-contained.

---

## Feature Summary

### 1. Presentation Mode (Default Expanded)
| Feature | Control | Result |
|---------|---------|--------|
| Start | Button | Begin presentation, reset timer |
| Pause | Button | Pause timer (pause != stop) |
| Resume | Button | Continue paused presentation |
| Stop | Button | End presentation, reset all |
| Timer | Display | MM:SS format, updates when running |
| Current Scenario | Display | Shows active scenario + step |
| Next Step | Button | Increment step counter |

### 2. Traffic Control (Default Expanded)
| Feature | Control | Range/Options |
|---------|---------|----------------|
| Rate | Slider | 1-100 ops/sec |
| Pattern | Buttons | uniform, zipf, burst |
| Pause Traffic | Toggle | Pauses generation |
| Resume Traffic | Toggle | Resumes generation |

### 3. Quick Actions (Default Collapsed)
| Action | Endpoint | Effect |
|--------|----------|--------|
| Populate Test Data | POST /test-data | Inserts 100 records |
| Clear Cache | POST /clear | Flushes all entries |
| Reset Statistics | POST /reset-stats | Clears metrics |
| Save State | Local File | Downloads JSON |
| Load State | File Upload | Restores from JSON |

### 4. Live Metrics (Default Expanded)
| Metric | Source | Update |
|--------|--------|--------|
| Ops/sec | /metrics | Every 1s |
| Memory MB | /metrics | Every 1s |
| Connections | /metrics | Every 1s |
| Hit Ratio % | Zustand Store | Real-time |
| Top Commands | /metrics | Every 1s |

### 5. Quick Launch (Default Collapsed)
| Scenario | Level | Starts |
|----------|-------|--------|
| Cold vs Warm | Beginner | cold-vs-warm |
| TTL & Expiration | Intermediate | ttl-expiration |
| Data Types | Intermediate | data-types |
| Eviction Policies | Advanced | eviction |

---

## API Endpoints Required

```
POST /traffic/rate              { rate: number }
POST /traffic/pattern           { pattern: 'uniform'|'zipf'|'burst' }
POST /traffic/pause
POST /traffic/resume
POST /test-data                 { count: number }
POST /clear
POST /reset-stats
GET  /metrics                   Returns metrics object
```

---

## State Interfaces

```typescript
// Presentation
{
  isRunning: boolean
  isPaused: boolean
  currentScenario: string
  currentStep: number
  elapsedSeconds: number
}

// Traffic
{
  rate: number (1-100)
  pattern: 'uniform' | 'zipf' | 'burst'
  isPaused: boolean
}

// Metrics Snapshot
{
  opsPerSecond: number
  memoryUsageMB: number
  activeConnections: number
  commandDistribution: { [cmd: string]: number }
  timestamp: number
}
```

---

## Keyboard Interactions

- **Tab**: Navigate between buttons
- **Enter/Space**: Activate button
- **Drag Header**: Move panel (grab icon)

---

## Styling Classes

All styling uses Tailwind + shadcn/ui. Respects theme variables:
- `bg-background`, `text-foreground`
- `bg-primary`, `bg-secondary`
- `hover:bg-accent`

---

## Component Structure

```
WorkshopControlPanel
├── Fixed Position Container
│   └── Card
│       ├── Draggable Header
│       │   ├── GripVertical Icon
│       │   ├── Title
│       │   └── Live Badge
│       └── CardContent (Scrollable)
│           ├── Presentation Section
│           ├── Traffic Section
│           ├── Quick Actions Section
│           ├── Live Metrics Section
│           └── Scenario Launch Section
```

---

## File I/O Operations

### Save State
```typescript
// Triggers browser download
// Filename: workshop-state-{timestamp}.json
// Content: Complete WorkshopState JSON
```

### Load State
```typescript
// File picker dialog
// Accepts: .json files
// Restores: all state, traffic, metrics
```

---

## Performance Notes

- Metrics history: Last 60 snapshots (1 minute rolling window)
- Polling interval: 1 second
- Panel max-height: 90vh (scrollable)
- Panel width: 384px (w-96)
- Z-index: 50

---

## Common Tasks

### Start Workshop
1. Click "Start" button
2. Select scenario from "Quick Launch"
3. Adjust traffic if needed
4. Click "Next Step" to progress

### Monitor Performance
Expand "Live Metrics" section to see:
- Current throughput (Ops/sec)
- Memory consumption
- Connection count
- Hit ratio
- Top commands

### Save for Later
1. Click "Save State"
2. Browser downloads JSON file
3. Keep for later reference

### Restore Previous State
1. Click "Load State"
2. Select previously saved JSON
3. All metrics and state restore

### Change Traffic Pattern
1. Expand "Traffic Control"
2. Click desired pattern (uniform/zipf/burst)
3. Adjust rate slider (1-100)
4. Changes apply immediately

---

## Debugging Checklist

- [ ] `SERVER_URL` config points to backend
- [ ] `/metrics` endpoint returns valid JSON
- [ ] `/traffic/*` endpoints respond with 200
- [ ] Zustand store has `hitRatio` property
- [ ] Component renders without console errors
- [ ] Panel is draggable from header only
- [ ] Metrics update every second
- [ ] Section collapse/expand works smoothly

---

## Browser Support

- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+
- (No IE11 support)

---

## Dependencies

- React 18+
- Next.js 13+ (App Router)
- Zustand (state management)
- shadcn/ui (UI components)
- Lucide React (icons)
- Tailwind CSS (styling)

---

## Color Theme

| Element | Color |
|---------|-------|
| Header | primary/10 to primary/5 gradient |
| Metrics | secondary/20 background |
| Scenario Info | primary/5 background |
| Active Button | primary (filled) |
| Inactive Button | outline |
| Destructive Button | red variant |

---

## File Sizes

- Component: ~15KB (minified)
- Documentation: ~25KB
- Total footprint: ~40KB

---

## Last Updated
November 8, 2025

## Version
1.0.0

## Status
Production Ready
