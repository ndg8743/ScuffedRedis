# Workshop Control Panel Component

A comprehensive floating, draggable control panel for managing Redis workshop presentations with real-time metrics, traffic control, and scenario management.

## File Location
`/Users/nathan/Downloads/ScuffedRedis/web/components/WorkshopControlPanel.tsx`

## Features

### 1. Presentation Mode Controls
- **Start/Stop/Pause**: Control presentation flow with dedicated buttons
- **Duration Timer**: Real-time MM:SS format timer tracking presentation length
- **Current Section Indicator**: Shows active scenario and step number
- **Next Step Button**: Step through scenarios sequentially

**State Management:**
- `PresentationState` tracks: `isRunning`, `isPaused`, `currentScenario`, `currentStep`, `elapsedSeconds`
- Timer runs on 1-second intervals when presentation is active and not paused

### 2. Traffic Control
- **Rate Slider**: Adjust operations per second (1-100 range)
- **Pattern Selection**: Choose between `uniform`, `zipf`, or `burst` traffic patterns
- **Pause/Resume**: Temporarily halt traffic generation without resetting
- **Real-time Updates**: Sends configuration to server via REST API

**Traffic Configuration:**
- Rate changes trigger immediate API calls to `/traffic/rate`
- Pattern changes trigger API calls to `/traffic/pattern`
- Pause/resume toggles `/traffic/pause` or `/traffic/resume` endpoints

### 3. Quick Actions
- **Populate Test Data**: Load 100 sample records into cache
- **Clear Cache**: Reset all cache entries (FLUSHDB equivalent)
- **Reset Statistics**: Clear all metric counters
- **Save State**: Download workshop state as JSON file
- **Load State**: Upload previously saved workshop state

**State Persistence:**
- Exports: presentation state, traffic config, metrics history
- Imports: restores complete workshop environment from JSON file
- Uses browser FileReader API for seamless file handling

### 4. Live Metrics Dashboard
Real-time monitoring with 60-second rolling window:

- **Operations Per Second**: Current throughput rate
- **Memory Usage**: Cache memory consumption in MB
- **Active Connections**: Number of connected clients
- **Hit Ratio**: Cache hit percentage (from store)
- **Command Distribution**: Top 5 most-used Redis commands with counts

**Metrics Collection:**
- Polls `/metrics` endpoint every 1 second
- Maintains rolling array of last 60 metrics snapshots
- Displays latest values with formatted decimals

### 5. Scenario Quick Launch
Pre-configured scenario buttons with difficulty badges:

1. **Cold vs Warm Cache** (Beginner)
   - Demonstrates cache miss penalties
   - Shows performance improvement with warming

2. **TTL and Expiration** (Intermediate)
   - Key expiration mechanics
   - Memory management strategies

3. **Data Types** (Intermediate)
   - Redis data structure operations
   - Performance characteristics

4. **Eviction Policies** (Advanced)
   - Memory eviction strategies
   - LRU vs LFU comparisons

Each button click:
- Sets scenario as current
- Resets step counter to 0
- Starts presentation automatically
- Resets timer

## Floating Panel Design

### Positioning & Dragging
- **Position**: Fixed 20px from top-left corner
- **Dimensions**: 384px width (w-96), 90vh max height
- **Dragging**: Grab handle in header (GripVertical icon)
- **Z-Index**: 50 (above most content, below modals)

**Dragging Implementation:**
```typescript
const [position, setPosition] = useState({ x: 20, y: 20 });
const [isDragging, setIsDragging] = useState(false);
const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });
```

- Mouse down on header activates drag mode
- Mouse move updates panel position
- Mouse up or leave ends dragging
- Cursor changes to `grabbing` during drag

### Collapsible Sections
Each major section can collapse/expand to manage space:
- **Presentation Mode**: Default expanded
- **Traffic Control**: Default expanded
- **Quick Actions**: Default collapsed
- **Live Metrics**: Default expanded
- **Scenario Quick Launch**: Default collapsed

Section state stored in `expandedSections` object with toggle function.

## API Endpoints

### Expected Server Endpoints

```typescript
// Traffic Control
POST /traffic/rate          // { rate: number }
POST /traffic/pattern       // { pattern: 'uniform' | 'zipf' | 'burst' }
POST /traffic/pause
POST /traffic/resume

// Quick Actions
POST /test-data             // { count: number }
POST /clear
POST /reset-stats

// Metrics
GET /metrics                // Returns { opsPerSecond, memoryUsageMB, activeConnections, commandDistribution }
```

## Component Usage

### Basic Import
```tsx
import { WorkshopControlPanel } from '@/components/WorkshopControlPanel';

export function Page() {
  return (
    <>
      <YourMainContent />
      <WorkshopControlPanel />
    </>
  );
}
```

The component is self-contained and manages its own state. No props required.

## State Interfaces

### PresentationState
```typescript
interface PresentationState {
  isRunning: boolean;         // Presentation active
  isPaused: boolean;          // Paused (not stopped)
  currentScenario: string;    // Scenario ID or 'none'
  currentStep: number;        // Step within scenario
  elapsedSeconds: number;     // Total seconds elapsed
}
```

### TrafficConfig
```typescript
interface TrafficConfig {
  rate: number;              // 1-100 ops/sec
  pattern: 'uniform' | 'zipf' | 'burst';
  isPaused: boolean;         // Traffic paused state
}
```

### MetricsSnapshot
```typescript
interface MetricsSnapshot {
  opsPerSecond: number;
  memoryUsageMB: number;
  activeConnections: number;
  commandDistribution: Record<string, number>;
  timestamp: number;
}
```

### WorkshopState (for save/load)
```typescript
interface WorkshopState {
  presentationState: PresentationState;
  trafficConfig: TrafficConfig;
  metrics: MetricsSnapshot[];
}
```

## Styling

Uses shadcn/ui components styled with Tailwind CSS:
- **Card**: Container with shadow and border
- **Button**: Multiple variants (default, outline, destructive)
- **Badge**: Status indicators and counts
- **Separator**: Visual dividers
- **Icons**: Lucide React icons throughout

### Color Scheme
- **Background**: Uses CSS variables (`bg-background`, `text-foreground`)
- **Accent**: `hover:bg-accent/50` for interactive areas
- **Primary**: Gradient header (`from-primary/10 to-primary/5`)
- **Secondary**: Metric boxes (`bg-secondary/20`)

## Performance Considerations

- **Metrics History**: Limited to 60 snapshots (1 minute of 1-second intervals)
- **Re-renders**: Optimized with React hooks and state management
- **API Calls**: Throttled to 1 request per second for metrics
- **File I/O**: Browser FileReader for save/load operations

## Integration Notes

### With AppStore (Zustand)
Component uses `hitRatio` from AppStore:
```typescript
const { hitRatio } = useAppStore();
// Displays in metrics as (hitRatio.ratio * 100).toFixed(1)%
```

### With Configuration
Server URL from config:
```typescript
import { SERVER_URL } from '@/lib/config';
```

## Common Workflows

### Starting a Workshop
1. Click "Start" in Presentation Mode
2. Select scenario from "Quick Launch" section
3. Use "Next Step" to progress through scenario
4. Adjust traffic rate/pattern as needed

### Saving Progress
1. Click "Save State" in Quick Actions
2. File downloads as `workshop-state-{timestamp}.json`
3. Contains full presentation state, traffic config, and metrics

### Resuming Previous State
1. Click "Load State" in Quick Actions
2. Select previously saved JSON file
3. Presentation, traffic, and metrics restore

### Monitoring Performance
Expand "Live Metrics" to see:
- Current operations per second
- Memory consumption
- Active connection count
- Hit ratio percentage
- Top 5 command distribution

## Accessibility Features

- Semantic HTML with proper button elements
- Icon labels in buttons via text content
- Keyboard navigation via native button focus
- Hover states on all interactive elements
- Color contrast ratios meet WCAG standards

## Browser Compatibility

- **Modern browsers** (Chrome, Firefox, Safari, Edge)
- **Requires**: ES2020+, CSS Grid, CSS Flex
- **No IE11 support** (uses modern React features)

## Troubleshooting

### Panel Not Dragging
Ensure you're clicking on the header area with the GripVertical icon. Other areas won't initiate drag.

### Metrics Not Updating
Check that `/metrics` endpoint is returning data in expected format:
```json
{
  "opsPerSecond": 42.5,
  "memoryUsageMB": 128.3,
  "activeConnections": 5,
  "commandDistribution": { "GET": 100, "SET": 50 }
}
```

### Traffic Changes Not Taking Effect
Verify API endpoints are responding with 200 status. Check browser console for fetch errors.

### State Load Fails
Ensure JSON file has correct structure matching `WorkshopState` interface. File must be valid JSON.
