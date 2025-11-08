# Enhanced 3D Heatmap Visualization - Usage Guide

## Quick Start

The visualization enhancements are automatically active. No additional configuration is needed beyond ensuring the backend sends event data with command types.

## Sending Events from Your Redis Backend

### Event Structure

Send events through Socket.IO with the following format:

```javascript
// Example: GET operation
socket.emit('cache_event', {
  id: 12345,                // Hash key for cell mapping
  hit: true,                // Cache hit (true) or miss (false)
  latency_ms: 2.3,          // Operation latency
  command: 'GET',           // Command type
  key: 'user:1001'          // Redis key (optional)
});

// Example: SET operation
socket.emit('cache_event', {
  id: 67890,
  hit: true,
  latency_ms: 1.1,
  command: 'SET',
  key: 'session:abc123'
});

// Example: DEL operation
socket.emit('cache_event', {
  id: 11111,
  hit: false,
  latency_ms: 0.8,
  command: 'DEL',
  key: 'temp:xyz'
});
```

### Supported Command Types

```typescript
type CommandType = 'GET' | 'SET' | 'DEL' | 'INCR' | 'EXPIRE' | 'OTHER';
```

| Command | Color  | Animation Pattern | Use Case |
|---------|--------|-------------------|----------|
| GET     | Green  | Smooth            | Read operations |
| SET     | Blue   | Bouncy            | Write/update operations |
| DEL     | Red    | Sharp spike       | Delete/eviction operations |
| INCR    | Yellow | Wave              | Counter/increment operations |
| EXPIRE  | Orange | Fade              | Key expiration events |
| OTHER   | Light Blue | Smooth      | Unknown/other operations |

## Visual Indicators

### Color Encoding

Each cell's color represents the most recent operation type:
- **Green** = GET (read)
- **Blue** = SET (write)
- **Red** = DEL (delete)
- **Yellow** = INCR (increment)
- **Orange** = EXPIRE (expiration)
- **Light Blue** = OTHER (unknown)

### Animation Patterns

Each operation type has a unique pulse pattern:

```
GET  (smooth):    ╭─╮
                 ╱   ╲

SET  (bouncy):   ╭╮╭╮
                ╱ ╲╱ ╲

DEL  (sharp):    │
                 │╲
                 │ ╲___

INCR (wave):    ╱╲╱╲╱╲
               ╱  ╲  ╲

EXPIRE (fade):  ╭──╮
               ╱    ╲
              ╱      ╲
```

### Operations Per Second Counter

The top right of the TopBar shows real-time throughput:

```
⚡ 125.3 ops/sec
```

This counter updates in real-time as operations flow through the system.

## Understanding the Heatmap

### Reading the Visualization

1. **Color**: Tells you what type of operation just occurred
2. **Animation**: The pulse pattern shows the operation type
3. **Location**: Determined by hashing the key ID (same key always lights up the same cell)
4. **Intensity**: Brightness indicates recent activity

### Cell Behavior Timeline

When an operation occurs:

1. **Immediate** (0-100ms): Cell color changes to operation color, animation starts
2. **Active** (100-900ms): Animation pulse plays out over ~900ms
3. **Fade** (900-2000ms): Color gradually fades back to neutral over ~1100ms
4. **Idle** (2000ms+): Cell returns to neutral gray

### Recognizing Patterns

**Read-Heavy Cache**:
- Predominantly green cells
- Smooth pulse patterns
- High ops/sec counter
- Consistent hit ratio

**Write-Heavy Workload**:
- Mix of green (GETs) and blue (SETs)
- Visible bounce patterns for writes
- Moderate ops/sec
- Pattern depends on write-to-read ratio

**Cache Eviction**:
- Red spikes appear periodically
- Sharp animation makes deletions obvious
- Usually correlated with memory pressure
- More frequent with smaller cache sizes

**Realistic Workload**:
- Mixed colors across the heatmap
- Different animation patterns visible simultaneously
- Ops/sec shows overall throughput
- Hit ratio indicates cache effectiveness

## Workshop Demonstration Scenarios

### Scenario 1: Understanding Operation Types

**Setup**:
1. Start with empty demonstration
2. Execute operations one at a time
3. Let each operation animate fully before next

**Commands**:
```bash
# Execute these manually to see different colors
GET user:1001       # See green smooth pulse
SET user:1002 data  # See blue bouncy pulse
DEL user:1003       # See red sharp spike
INCR counter:1      # See yellow wave
EXPIRE key:1 60     # See orange fade
```

**Talking Points**:
- Each operation type has a distinctive color and animation
- The animation pattern communicates the operation type instantly
- Size and intensity of pulse shows operation importance

### Scenario 2: Cache Hit Performance

**Setup**:
1. Warm up cache with initial data
2. Run read-heavy workload (50+ GET operations)
3. Monitor hit ratio and colors

**Observations**:
- Predominantly green heatmap
- Hit ratio badge shows cache effectiveness
- Smooth animations indicate read patterns
- Ops/sec counter shows read throughput

**Talking Points**:
- Cache hits are visible as green operations
- Consistent green indicates good cache locality
- Hit ratio trends upward with more reads

### Scenario 3: Mixed Workload Behavior

**Setup**:
1. Send mixed operations (30% GET, 50% SET, 20% other)
2. Observe heatmap for several seconds
3. Monitor ops/sec and hit ratio

**Observations**:
- Mix of colors across the heatmap
- Blue (SET) less frequent than green (GET)
- Various animation patterns visible simultaneously
- Ops/sec shows total throughput

**Talking Points**:
- Real applications have mixed workloads
- Different operation types are easily distinguished visually
- Animation patterns provide intuitive communication

### Scenario 4: Cache Eviction Demonstration

**Setup**:
1. Set small cache size
2. Fill cache completely
3. Add more keys than fit
4. Watch eviction events

**Observations**:
- Red DEL operations appear
- Sharp spike animation is very distinctive
- Correlates with cache size constraints
- Eviction pattern follows configured policy

**Talking Points**:
- Red spikes indicate cache cleanup
- Frequency shows eviction policy in action
- Helps explain memory management tradeoffs

## Customization

### Adjusting Animation Speed

Edit `/web/lib/config.ts`:

```typescript
export const CONFIG = {
  // Faster animations (ms)
  PULSE_DURATION: 600,           // Was 900
  COLOR_FADE_DURATION: 1200,     // Was 2000
```

- Lower values = faster animations
- Higher values = slower, more dramatic animations

### Changing Colors

Edit `/web/lib/config.ts`:

```typescript
COMMAND_COLORS: {
  GET: [0, 1, 0],      // [R, G, B] in 0-1 range
  SET: [0, 0.5, 1],    // Changed from pure blue
  // ... more colors
}
```

Color format is RGB with values from 0 to 1.

### Modifying Animation Patterns

The patterns are defined in `/web/components/Heatmap.tsx` in the `useFrame` hook:

```typescript
switch (cellState.pulsePattern) {
  case 'custom':
    // Add your own formula here
    pulseScale = 1 + Math.sin(pulseProgress * Math.PI * 4) * 0.25;
    break;
}
```

Then add to config:
```typescript
ANIMATION_PATTERNS: {
  'GET': 'custom',
  // ...
}
```

## Integration with Monitoring

The visualization can complement traditional monitoring:

### Complement Metrics

| Visualization | Traditional Monitoring |
|---------------|----------------------|
| Visual operation types | Command histograms |
| Real-time ops/sec | Metrics dashboard |
| Hit ratio indicator | Cache hit % metrics |
| Spatial patterns | Key distribution stats |

### Using Both Together

1. **Visualization first**: Get immediate visual intuition
2. **Metrics second**: Verify with numerical data
3. **Alerts third**: Set thresholds from understanding

## Performance Considerations

### Optimal Configurations

**High Throughput (1000+ ops/sec)**:
- Larger heatmap (30x30)
- Faster animation durations
- More visible color variation

**Demonstration Mode (10-100 ops/sec)**:
- Standard 20x20 heatmap
- Moderate animation durations
- Clear visual separation

**Low Throughput (<10 ops/sec)**:
- Smaller heatmap (15x15)
- Slower animation durations
- Allow more fade-out visibility

### Browser Requirements

- Chrome/Edge: Full support
- Safari: Full support with WebGL
- Firefox: Full support with WebGL
- Mobile: Works but performance depends on device

## Troubleshooting

### Colors Not Changing

**Check**:
1. Backend is sending `command` field
2. Command type matches one of: GET, SET, DEL, INCR, EXPIRE, OTHER
3. Events are being received (check Network tab)

**Fix**:
```javascript
// Ensure your backend sends:
socket.emit('cache_event', {
  ...event,
  command: 'GET'  // Must be included
});
```

### Animations Too Fast/Slow

**Adjust in config.ts**:
```typescript
PULSE_DURATION: 900,           // Increase to 1500 for slower
COLOR_FADE_DURATION: 2000,     // Increase to 3000 for longer fade
```

### Ops/Sec Counter Not Updating

**Check**:
1. Events are being sent to frontend
2. TopBar component has access to `operationsPerSecond` state
3. Page is calculating ops/sec from event timestamps

**Debug**:
```javascript
// In browser console
import { useAppStore } from '@/lib/state';
const store = useAppStore.getState();
console.log(store.operationsPerSecond);
console.log(store.events);
```

### Legend Not Showing Commands

**Check**:
1. Legend component is rendered in page.tsx
2. CSS classes exist in stylesheet
3. No CSS conflicts hiding the legend

**Fix**:
```bash
npm run dev  # Restart development server
```

## Advanced Usage

### Real-Time Workload Analysis

Use the visualization to understand your actual workload:

1. Run production traffic against visualization
2. Observe color distribution over 5-10 minutes
3. Note spike patterns and anomalies
4. Correlate with performance metrics

### Load Testing Visualization

During load tests:

1. Watch ops/sec counter reach peak
2. Observe hit ratio trend
3. Monitor color distribution for command mix
4. Identify bottleneck operations (longest animations)

### Training and Demos

For teaching Redis concepts:

1. **Caching**: Show GET operations and hit ratio
2. **Updates**: Show SET operations with blue color
3. **Eviction**: Trigger DEL and watch red spikes
4. **Load**: Show ops/sec increasing with traffic
5. **Patterns**: Discuss spatial distribution of operations

## Key Takeaways

1. **Colors = Operation Types**: Green for GET, Blue for SET, Red for DEL, etc.
2. **Animations = Visual Feedback**: Different patterns communicate operation characteristics
3. **Ops/Sec = Real-Time Throughput**: Single metric showing system load
4. **Legend = Reference Guide**: Always available for understanding encoding
5. **Spatial Layout = Key Distribution**: Same key always maps to same cell

## Next Steps

1. Configure backend to send command types
2. Customize colors/animations to your preference
3. Use visualization in demonstrations and training
4. Correlate visual patterns with system metrics
5. Iteratively improve based on user feedback
