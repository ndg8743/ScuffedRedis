# 3D Heatmap Visualization Enhancements

## Overview

The Redis Cache Heatmap visualization has been enhanced with advanced visual feedback and command type indicators to provide a more engaging and informative workshop demonstration experience.

## Features Implemented

### 1. Command Type Indicators with Dynamic Colors

Each Redis command type is assigned a distinct color that pulses when that command is executed:

- **GET** (Green): Read operations, smooth pulse pattern
- **SET** (Blue): Write operations, bouncy pulse pattern
- **DEL** (Red): Delete operations, sharp spike pattern
- **INCR** (Yellow): Increment operations, wave pulse pattern
- **EXPIRE** (Orange): Key expiration operations, fade pulse pattern
- **OTHER** (Light Blue): Unknown/other operations, smooth pulse pattern

Colors are mapped in `/web/lib/config.ts`:
```typescript
COMMAND_COLORS: {
  GET: [0, 1, 0],      // Green
  SET: [0, 0, 1],      // Blue
  DEL: [1, 0, 0],      // Red
  INCR: [1, 1, 0],     // Yellow
  EXPIRE: [1, 0.5, 0], // Orange
  OTHER: [0.5, 0.5, 1] // Light blue
}
```

### 2. Enhanced Animation Patterns

Different Redis commands trigger unique animation patterns to provide visual distinction:

- **Smooth Pulse** (GET, OTHER): Gentle sine wave expansion, 20% scale increase
- **Bouncy Pulse** (SET): Dual-frequency sine wave, 15% scale with rapid oscillation
- **Sharp Spike** (DEL): Immediate 30% spike with quick decay, indicates destructive operations
- **Wave Pulse** (INCR): Multi-frequency wave with decay, 20% max scale
- **Fade Pulse** (EXPIRE): Subtle 10% variation without aggressive scaling

Animation patterns are defined in `/web/lib/config.ts`:
```typescript
ANIMATION_PATTERNS: {
  GET: 'smooth',     // Smooth sine wave
  SET: 'bounce',     // Bouncy dual pulse
  DEL: 'sharp',      // Sharp spike
  INCR: 'wave',      // Wave with decay
  EXPIRE: 'fade',    // Fade pulse
  OTHER: 'smooth'    // Smooth pulse
}
```

### 3. Real-Time Operations Per Second Counter

An operations per second (ops/sec) counter is displayed in the top bar, showing:
- Current throughput measured over a rolling 1-second window
- Animated pulse icon indicator
- Updates in real-time as operations flow through the system

The counter is calculated in `/web/app/page.tsx` by:
1. Filtering events from the last 1000ms
2. Counting the total number of operations
3. Displaying with one decimal place precision

Display in TopBar:
```
[Lightning Bolt Icon] 125.3 ops/sec
```

### 4. Visual Legend and Documentation

The Legend component (`/web/components/Legend.tsx`) has been enhanced to display:
- Command type color indicators
- Associated animation pattern names
- Hit ratio statistics
- Total request count

This provides attendees with a quick visual reference for understanding the visualization.

## Technical Implementation Details

### State Management (state.ts)

Extended the Zustand store with:

```typescript
export type CommandType = 'GET' | 'SET' | 'DEL' | 'INCR' | 'EXPIRE' | 'OTHER';

export interface CacheEvent {
  id: number;
  hit: boolean;
  latency_ms: number;
  timestamp: number;
  command?: CommandType;  // New field for command type
  key?: string;           // New field for key name
}

// New state properties:
commandStats: CommandStats;
operationsPerSecond: number;
updateOpsPerSecond: (ops: number) => void;
```

### Heatmap Component (Heatmap.tsx)

Enhanced with:

1. **Command-Aware Cell State**:
   - Tracks command type, key name, and pulse pattern for each cell
   - Stores target and current colors separately for smooth interpolation

2. **Pattern-Specific Animation Logic**:
   - Each animation pattern uses different mathematical functions
   - Sharp spike uses linear decay: `1 + max(0, 1-progress) * 0.3`
   - Bouncy uses high-frequency sine: `sin(progress * PI * 2)`
   - Wave uses multi-frequency with decay: `sin(progress * PI * 3) * (1 - progress)`

3. **Color Interpolation**:
   - Smooth lerp to target color (10% per frame)
   - Separate color fade timeline from scale animation
   - Smooth return to neutral when timers expire

### TopBar Integration

Display real-time metrics:
- Operations per second with pulsing icon
- Maintains all existing functionality
- Non-intrusive visual indicator

## Workshop Demonstration Use Cases

### Cache Read Performance
- Observe green GET operations accumulating
- Watch smooth pulse patterns indicating read-heavy workload
- Compare with occasional SET operations in blue

### Write-Heavy Operations
- Monitor blue SET operations with bouncy patterns
- Demonstrates update frequency and distribution
- Shows cache write throughput in real-time

### Cache Eviction Events
- Red DEL operations with sharp spike animations
- Immediately visible cache cleanup activities
- Helps explain eviction policies visually

### Command Mix Analysis
- Multi-colored heatmap shows realistic command distribution
- Each color represents a cache operation type
- Attendees can understand typical workload patterns

## Performance Considerations

- **Instanced Rendering**: Uses Three.js InstancedMesh for 400 cells with minimal overhead
- **Efficient Animation**: Pattern calculations done in animation loop with minimal allocations
- **Event Filtering**: Operations per second uses efficient filter on last 100 events
- **Color Updates**: Single instanceColor update per frame via Three.js

## Configuration

All visual parameters are centralized in `/web/lib/config.ts`:

```typescript
// Animation duration settings
PULSE_DURATION: 900,           // ms for pattern animation
COLOR_FADE_DURATION: 2000,     // ms for color fade-out

// All colors in RGB [0,1] format
COMMAND_COLORS: { ... }

// All animation pattern names
ANIMATION_PATTERNS: { ... }
```

Modify these values to adjust visual feedback timing and intensity.

## Server Integration

For full functionality, the backend server should send events with:

```json
{
  "id": 12345,
  "hit": true,
  "latency_ms": 2.3,
  "command": "GET",
  "key": "user:1001"
}
```

The visualization will gracefully handle missing command/key fields by using 'OTHER' and generic key names.

## Files Modified

1. **web/lib/state.ts** - Extended Zustand store with command tracking
2. **web/lib/config.ts** - Added command colors and animation patterns
3. **web/components/Heatmap.tsx** - Enhanced with pattern-specific animations
4. **web/components/TopBar.tsx** - Added operations per second display
5. **web/components/Legend.tsx** - Enhanced with command type indicators
6. **web/app/page.tsx** - Added real-time ops/sec calculation

## Future Enhancement Opportunities

- Hover tooltips showing key name and operation details
- Cell size variation based on operation latency
- Animation speed variation based on command type
- Statistical breakdown of operations by type
- Custom color themes for different scenarios
