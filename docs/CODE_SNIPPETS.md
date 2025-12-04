# 3D Heatmap Enhancement - Code Snippets Reference

## Quick Reference for Implementation

### 1. Command Type Colors - config.ts

```typescript
// Location: /web/lib/config.ts
export const CONFIG = {
  // ... existing config ...

  // Command colors - specific operation type colors
  COMMAND_COLORS: {
    GET: [0, 1, 0],      // Green
    SET: [0, 0, 1],      // Blue
    DEL: [1, 0, 0],      // Red
    INCR: [1, 1, 0],     // Yellow
    EXPIRE: [1, 0.5, 0], // Orange
    OTHER: [0.5, 0.5, 1] // Light blue
  },

  // Animation patterns for different commands
  ANIMATION_PATTERNS: {
    GET: 'smooth',     // Smooth pulse
    SET: 'bounce',     // Bouncy pulse
    DEL: 'sharp',      // Sharp spike
    INCR: 'wave',      // Wave pulse
    EXPIRE: 'fade',    // Fade pulse
    OTHER: 'smooth'    // Smooth pulse
  }
} as const;
```

---

### 2. Command Type Definition - state.ts

```typescript
// Location: /web/lib/state.ts

export type CommandType = 'GET' | 'SET' | 'DEL' | 'INCR' | 'EXPIRE' | 'OTHER';

export interface CacheEvent {
  id: number;
  hit: boolean;
  latency_ms: number;
  timestamp: number;
  command?: CommandType;  // NEW: Operation type
  key?: string;           // NEW: Redis key
}

export interface CommandStats {
  GET: number;
  SET: number;
  DEL: number;
  INCR: number;
  EXPIRE: number;
  OTHER: number;
}

interface AppState {
  // ... existing state ...

  // Command stats
  commandStats: CommandStats;
  operationsPerSecond: number;
  updateOpsPerSecond: (ops: number) => void;
}
```

---

### 3. Enhanced Cell State - Heatmap.tsx

```typescript
// Location: /web/components/Heatmap.tsx

interface CellState {
  colorTimer: number;              // Time remaining for color animation
  pulseTimer: number;              // Time remaining for pulse animation
  targetColor: [number, number, number];  // Target RGB color
  currentColor: [number, number, number]; // Current RGB color
  scale: number;                   // Current scale factor
  targetScale: number;             // Target scale factor
  pulsePattern: string;            // Animation pattern name
  keyName: string;                 // Redis key name
  command: CommandType;            // Operation type
}
```

---

### 4. Color Selection Logic - Heatmap.tsx

```typescript
// Location: /web/components/Heatmap.tsx

// Get color based on command type
const getCommandColor = (command: CommandType): [number, number, number] => {
  return CONFIG.COMMAND_COLORS[command] || CONFIG.COMMAND_COLORS.OTHER;
};

// Process new events with command type awareness
useEffect(() => {
  if (events.length === 0) return;

  const latestEvent = events[events.length - 1];
  const cellIndex = getCellIndex(latestEvent.id, CONFIG.TOTAL_CELLS);
  const cellState = cellStates[cellIndex];

  if (cellState) {
    cellState.command = latestEvent.command || 'OTHER';
    cellState.keyName = latestEvent.key || `Key ${latestEvent.id}`;
    cellState.pulsePattern = CONFIG.ANIMATION_PATTERNS[cellState.command];

    // Use command-specific color
    cellState.targetColor = getCommandColor(cellState.command);

    // Enhance scale based on animation pattern
    switch (cellState.pulsePattern) {
      case 'sharp':
        cellState.targetScale = 1.3; // DEL gets larger spike
        break;
      case 'bounce':
        cellState.targetScale = 1.15; // SET gets moderate bounce
        break;
      case 'wave':
        cellState.targetScale = 1.2; // INCR gets wave motion
        break;
      default:
        cellState.targetScale = 1.15;
    }

    // Reset timers
    cellState.colorTimer = CONFIG.COLOR_FADE_DURATION;
    cellState.pulseTimer = CONFIG.PULSE_DURATION;
  }
}, [events, cellStates]);
```

---

### 5. Pattern-Specific Animation - Heatmap.tsx

```typescript
// Location: /web/components/Heatmap.tsx - useFrame hook

// Update pulse animation with pattern variations
if (cellState.pulseTimer > 0) {
  cellState.pulseTimer -= deltaMs;

  const pulseProgress = 1 - (cellState.pulseTimer / CONFIG.PULSE_DURATION);

  // Different pulse patterns for different operations
  let pulseScale = 1;
  switch (cellState.pulsePattern) {
    case 'smooth':
      // Smooth sine wave - GET, OTHER
      pulseScale = 1 + Math.sin(pulseProgress * Math.PI) * 0.2;
      break;

    case 'bounce':
      // Bouncy dual pulse - SET
      pulseScale = 1 + Math.sin(pulseProgress * Math.PI * 2) * 0.15;
      break;

    case 'sharp':
      // Quick sharp spike - DEL
      pulseScale = 1 + Math.max(0, 1 - pulseProgress) * 0.3;
      break;

    case 'wave':
      // Wave with decay - INCR
      pulseScale = 1 + Math.sin(pulseProgress * Math.PI * 3) * (0.2 * (1 - pulseProgress));
      break;

    case 'fade':
      // Fade without size change - EXPIRE
      pulseScale = 1 + Math.cos(pulseProgress * Math.PI) * 0.1;
      break;

    default:
      pulseScale = 1 + Math.sin(pulseProgress * Math.PI) * 0.2;
  }

  cellState.scale = pulseScale;
} else {
  // Return to normal scale
  cellState.scale += (1 - cellState.scale) * 0.1;
}
```

---

### 6. Color Interpolation - Heatmap.tsx

```typescript
// Location: /web/components/Heatmap.tsx - useFrame hook

// Update color animation
if (cellState.colorTimer > 0) {
  cellState.colorTimer -= deltaMs;

  // Smooth lerp to target color
  const t = 0.1;
  cellState.currentColor = [
    cellState.currentColor[0] + (cellState.targetColor[0] - cellState.currentColor[0]) * t,
    cellState.currentColor[1] + (cellState.targetColor[1] - cellState.currentColor[1]) * t,
    cellState.currentColor[2] + (cellState.targetColor[2] - cellState.currentColor[2]) * t,
  ] as [number, number, number];

  // Fade back to neutral when timer expires
  if (cellState.colorTimer <= 0) {
    cellState.targetColor = CONFIG.COLORS.NEUTRAL;
  }
} else {
  // Fade to neutral
  const t = 0.05;
  cellState.currentColor = [
    cellState.currentColor[0] + (CONFIG.COLORS.NEUTRAL[0] - cellState.currentColor[0]) * t,
    cellState.currentColor[1] + (CONFIG.COLORS.NEUTRAL[1] - cellState.currentColor[1]) * t,
    cellState.currentColor[2] + (CONFIG.COLORS.NEUTRAL[2] - cellState.currentColor[2]) * t,
  ] as [number, number, number];
}

// Update instance color
const color = new Color(
  cellState.currentColor[0],
  cellState.currentColor[1],
  cellState.currentColor[2]
);
meshRef.current.setColorAt(index, color);
```

---

### 7. Operations Per Second Display - TopBar.tsx

```typescript
// Location: /web/components/TopBar.tsx

export function TopBar({ onOpenCommandPlayground, ...other props }: TopBarProps) {
  const { hitRatio, isConnected, operationsPerSecond } = useAppStore();
  // ... other code ...

  return (
    <div className="flex items-center justify-between p-4 bg-background/80 backdrop-blur-sm border-b">
      {/* ... other content ... */}

      <div className="flex items-center space-x-4">
        {/* ... hit ratio ... */}

        {/* NEW: Operations per second display */}
        <div className="flex items-center space-x-2">
          <Zap className="w-4 h-4 text-yellow-500 animate-pulse" />
          <span className="text-sm font-medium">{operationsPerSecond.toFixed(1)} ops/sec</span>
        </div>

        {/* ... buttons ... */}
      </div>
    </div>
  );
}
```

---

### 8. Operations Per Second Calculation - page.tsx

```typescript
// Location: /web/app/page.tsx

export default function HomePage() {
  const { updateHitRatio, events, updateOpsPerSecond } = useAppStore();
  // ... other state ...

  useEffect(() => {
    setMounted(true);
    // ... socket setup and hit ratio polling ...
  }, [updateHitRatio, updateOpsPerSecond]);

  // NEW: Calculate operations per second from events
  useEffect(() => {
    if (events.length < 2) return;

    const now = Date.now();
    const oneSecondAgo = now - 1000;

    // Count events in the last second
    const recentEvents = events.filter(e => e.timestamp >= oneSecondAgo);
    const opsPerSecond = recentEvents.length;

    updateOpsPerSecond(opsPerSecond);
  }, [events, updateOpsPerSecond]);

  // ... rest of component ...
}
```

---

### 9. Enhanced Legend - Legend.tsx

```typescript
// Location: /web/components/Legend.tsx

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
```

---

### 10. Example Event from Backend

```json
{
  "id": 12345,
  "hit": true,
  "latency_ms": 2.3,
  "command": "GET",
  "key": "user:1001"
}
```

```json
{
  "id": 67890,
  "hit": true,
  "latency_ms": 1.1,
  "command": "SET",
  "key": "session:abc123"
}
```

```json
{
  "id": 11111,
  "hit": false,
  "latency_ms": 0.8,
  "command": "DEL",
  "key": "temp:xyz"
}
```

---

## Implementation Checklist

### For Backend Team

- [ ] Send `command` field in cache_event
- [ ] Send `key` field in cache_event
- [ ] Ensure command is one of: GET, SET, DEL, INCR, EXPIRE, OTHER
- [ ] Ensure key field matches actual Redis key
- [ ] Test with high-throughput scenario (1000+ ops/sec)
- [ ] Test with mixed workload (multiple command types)

### For Frontend Team

- [ ] Verify colors appear correctly for each command type
- [ ] Test animation patterns are distinct
- [ ] Verify ops/sec counter updates in real-time
- [ ] Check legend displays correctly
- [ ] Test with missing command field (should default to OTHER)
- [ ] Test with missing key field (should use generic label)
- [ ] Verify no performance degradation

### For QA Team

- [ ] Test all command types individually
- [ ] Test mixed workload with all types simultaneously
- [ ] Verify animations don't stutter or lag
- [ ] Check colors are accurate to specification
- [ ] Verify legend matches implementation
- [ ] Test with 1000+ ops/sec throughput
- [ ] Test with 100+ ops/sec for extended periods
- [ ] Verify memory usage is stable

---

## Mathematical Formulas Reference

### Smooth Pulse (GET, OTHER)
```
scale = 1 + sin(progress * π) * 0.2
Range: [1.0, 1.2]
Duration: 900ms
Effect: Gentle one-cycle sine wave
```

### Bouncy Pulse (SET)
```
scale = 1 + sin(progress * π * 2) * 0.15
Range: [0.85, 1.15]
Duration: 900ms
Effect: Two cycles of oscillation
```

### Sharp Spike (DEL)
```
scale = 1 + max(0, 1-progress) * 0.3
Range: [1.0, 1.3]
Duration: 900ms
Effect: Linear decay from peak to baseline
```

### Wave Pulse (INCR)
```
scale = 1 + sin(progress * π * 3) * (0.2 * (1-progress))
Range: [1.0, 1.2] with decay
Duration: 900ms
Effect: Three-cycle wave with envelope decay
```

### Fade Pulse (EXPIRE)
```
scale = 1 + cos(progress * π) * 0.1
Range: [0.9, 1.1]
Duration: 900ms
Effect: Subtle cosine wave
```

---

## File Locations Summary

| Feature | File | Lines Added |
|---------|------|-------------|
| Command Colors | config.ts | 19 |
| Command Types | state.ts | 22 |
| Heatmap Logic | Heatmap.tsx | ~100 |
| Ops/Sec Display | TopBar.tsx | ~10 |
| Legend Update | Legend.tsx | ~40 |
| Ops/Sec Calc | page.tsx | ~15 |
| **Total** | | ~206 |

---

## Color Reference (RGB Values)

| Command | Color | RGB | Hex |
|---------|-------|-----|-----|
| GET | Green | [0, 1, 0] | #00FF00 |
| SET | Blue | [0, 0, 1] | #0000FF |
| DEL | Red | [1, 0, 0] | #FF0000 |
| INCR | Yellow | [1, 1, 0] | #FFFF00 |
| EXPIRE | Orange | [1, 0.5, 0] | #FF8000 |
| OTHER | Light Blue | [0.5, 0.5, 1] | #8080FF |
| NEUTRAL | Gray | [0.3, 0.3, 0.3] | #4D4D4D |

---

## Animation Duration Reference

| Setting | Default | Recommendation |
|---------|---------|-----------------|
| PULSE_DURATION | 900ms | 600-1200ms |
| COLOR_FADE_DURATION | 2000ms | 1500-2500ms |
| Animation Speed | 60fps | Target for smooth motion |

---

## Integration Testing Checklist

```bash
# Before deployment, verify:
[ ] npm run build succeeds
[ ] No TypeScript errors
[ ] Heatmap renders without console errors
[ ] Colors update on events
[ ] Animations play smoothly
[ ] Ops/sec counter updates
[ ] Legend displays correctly
[ ] No memory leaks after 1 hour
[ ] Works at 1000+ ops/sec
[ ] Gracefully handles missing fields
```

