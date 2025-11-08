# Workshop Guide - 1 Hour Presentation

This guide shows how to use all the integrated workshop components for an effective 1-hour Redis caching presentation.

## Quick Start

### Development Mode (Full Stack)
```bash
npm run workshop
```
This starts:
- ScuffedRedis C++ server on port 6379
- Node.js backend on port 4000
- React frontend on port 3000

### Demo Mode (Without C++)
```bash
npm run workshop:demo
```
This starts just the server and web frontend, assuming ScuffedRedis is already running.

## UI Components Overview

All components are accessible from the top navigation bar:

### 1. Command Playground
Button: **Command**
- Execute Redis commands manually
- See results in real-time
- Useful for manual demonstrations

### 2. Educational Popups
Button: **Learn**
- 8 interactive educational modules covering:
  - How Redis caching works (Cache-Aside Pattern)
  - Cache Memory & TTL Management
  - Performance Metrics & Hit Ratio
  - Data Structures & Operations
  - Advanced Cache Patterns
  - Real-world Use Cases
  - Cache Eviction Strategies
  - Troubleshooting Common Issues

### 3. Workshop Scenarios
Button: **Scenarios**
- 4 Pre-designed learning scenarios:

#### Scenario 1: Cold vs Warm Cache (10 min, Beginner)
**Objectives:**
- Understand difference between cold and warm cache states
- See impact on hit ratio

**Steps:**
1. Warm up 50 items
2. Monitor hit ratio increase
3. Demonstrate first-access misses vs. subsequent hits
4. Show latency differences

#### Scenario 2: Traffic Patterns Impact (15 min, Intermediate)
**Objectives:**
- Learn how traffic patterns affect cache performance
- Compare constant vs. spike patterns

**Steps:**
1. Start with constant traffic pattern
2. Observe steady hit ratio
3. Switch to spike pattern
4. Show cache response to traffic bursts

#### Scenario 3: Operation Type Mix (12 min, Intermediate)
**Objectives:**
- Understand impact of read vs. write operations
- Learn about cache invalidation

**Steps:**
1. Start with read-heavy operations
2. Transition to mixed operations
3. Observe cache invalidation during writes
4. Show hit ratio stabilization

#### Scenario 4: Performance Tuning (23 min, Advanced)
**Objectives:**
- Optimize cache for specific workloads
- Balance performance metrics

**Steps:**
1. Adjust traffic rate and patterns
2. Monitor operations per second
3. Fine-tune for maximum hit ratio
4. Demonstrate trade-offs

### 4. Workshop Control Panel
Button: **Control**

#### Traffic Control Section
- **Operations/Second**: Adjust traffic rate (1-100 ops/sec)
- **Pattern**: Select traffic pattern
  - Constant: Steady traffic
  - Spike: Sudden bursts (10% of time at 10x rate)
  - Wave: Sinusoidal pattern
  - Random: Completely random
- **Operation Type**:
  - Read Heavy: 70% GET, 20% SET, 10% DEL
  - Write Heavy: 20% GET, 70% SET, 10% DEL
  - Mixed: 70% GET, 20% SET, 10% DEL
- **Control Buttons**:
  - Start: Begin traffic generation
  - Pause: Temporarily stop traffic
  - Stop: Stop and clear cache

#### Performance Metrics
- Current operations per second
- Current hit ratio percentage
- Reset Statistics button

#### Checkpoint Management
- Save current state as checkpoint
- View all saved checkpoints
- Load any previous checkpoint
- Automatically persisted in localStorage

## 60-Minute Presentation Schedule

### 0-5 minutes: Introduction
- Explain Redis cache fundamentals
- Show the 3D visualization
- Demonstrate how green (hits) and red (misses) cells work

### 5-15 minutes: Cold vs Warm Cache (Scenario 1)
- Click "Scenarios" button
- Select "Cold vs Warm Cache"
- Walk through guided steps
- Highlight hit ratio changes
- Show that first requests miss, subsequent ones hit

### 15-30 minutes: Educational Content
- Click "Learn" button
- Go through "How Redis Caching Works"
- Explain the Cache-Aside Pattern
- Show typical latency numbers (cache vs database)
- Cover cache memory and TTL

### 30-45 minutes: Traffic Patterns (Scenario 2)
- Go back to Scenarios
- Select "Traffic Patterns Impact"
- Use Control Panel to adjust patterns
- Start with constant traffic (20 ops/sec)
- Switch to spike pattern
- Show how cache responds to bursts

### 45-55 minutes: Advanced Topics
- Run Scenario 3 (Operation Type Mix)
- Explain cache invalidation
- Show impact of write operations
- Discuss cache coherency

### 55-60 minutes: Q&A and Review
- Use Control Panel to save final state
- Answer audience questions
- Show how to reset and try different configurations

## Control Panel Tips

### Saving Checkpoints
Perfect for capturing key moments:
```
1. Set up interesting traffic pattern
2. Click "Save Current State as Checkpoint"
3. Give it a descriptive name
4. Can return to exact state later
```

### Traffic Patterns for Demos

**Constant (Predictable)**
```
- Best for explaining baseline behavior
- Steady hit ratio improvements
- Easy to predict and explain
```

**Spike (Real-world)**
```
- Shows cache resilience
- Dramatic visuals with sudden activity
- Good for showing burst handling
```

**Wave (Gradual)**
```
- Shows smooth transitions
- Like business hour patterns
- Good for long-term observation
```

**Random (Chaos)**
```
- Unpredictable behavior
- Best for advanced discussion
- Shows cache under random access
```

## Integration Architecture

### New API Endpoints

**POST /workshop/traffic**
- Controls traffic generation
- Parameters: action (start/pause/stop), config
- Response: status, current config

**POST /workshop/reset-stats**
- Resets all statistics to zero
- Response: status

**POST /workshop/save-state**
- Saves current state as checkpoint
- Parameters: trafficConfig, hitRatio, commandStats, scenarioName
- Response: saved state with timestamp

**POST /workshop/load-state**
- Restores a saved checkpoint
- Parameters: trafficConfig, scenarioName
- Response: status, scenarioName

**GET /workshop/states**
- Lists all saved checkpoints
- Response: array of states

### Socket.IO Events

**cache_event** (enhanced)
- Includes: id, hit, latency_ms, operationType, timestamp, cacheStats
- Sent on each cache operation

**stats_reset**
- Sent when statistics are reset
- Contains: hits, misses, ratio

**state_loaded**
- Sent when a checkpoint is loaded
- Contains: scenarioName

**initial_stats**
- Sent on client connection
- Contains: current cache statistics

## Troubleshooting

### Workshop not starting?
1. Ensure ports 3000, 4000, 6379 are free
2. Check Redis server is running: `npm run dev:cpp`
3. Check backend: `npm run dev:server`
4. Check frontend: `npm run dev:web`

### Traffic not generating?
1. Click "Start" in Control Panel
2. Check browser console for errors
3. Ensure socket connection is established (check TopBar)
4. Try resetting stats

### States not saving?
1. Check localStorage is enabled in browser
2. Look for errors in browser DevTools
3. Try refreshing the page
4. Save again with a different name

## Advanced Tips

### Multi-Monitor Setup
- Run on display 1: React frontend (http://localhost:3000)
- Run on display 2: Control Panel
- Presenter can manage traffic while audience watches visualization

### Recording Presentations
1. Use screen recording software
2. Start with "Warm Up" for cache priming
3. Record scenarios in order
4. Good for asynchronous sharing

### Custom Workflows
Create your own scenarios:
1. Start traffic with specific pattern
2. Let it run to steady state
3. Save as checkpoint
4. Load during presentation at key moments
5. Use for A/B comparisons

## Key Metrics to Watch

### Hit Ratio
- 0-20%: Cold cache, warming up
- 20-50%: Warming, early optimization phase
- 50-80%: Good cache performance
- 80%+: Excellent, well-optimized cache

### Operations per Second
- Current throughput indicator
- Shows traffic pattern effectiveness
- Helps explain performance impact

### Animation Latency
- Green flashes: Cache hits (~1ms)
- Red flashes: Cache misses (~100ms+)
- Visual difference shows performance benefit

## Component Files

### Frontend Components
- `/Users/nathan/Downloads/ScuffedRedis/web/components/WorkshopControlPanel.tsx` - Control traffic and states
- `/Users/nathan/Downloads/ScuffedRedis/web/components/WorkshopScenarios.tsx` - Guided learning scenarios
- `/Users/nathan/Downloads/ScuffedRedis/web/components/EducationalPopups.tsx` - Educational content

### Backend
- `/Users/nathan/Downloads/ScuffedRedis/server/src/index.ts` - API endpoints (lines 158-252)
- `/Users/nathan/Downloads/ScuffedRedis/server/src/traffic.ts` - Traffic generation with patterns

### State Management
- `/Users/nathan/Downloads/ScuffedRedis/web/lib/state.ts` - Workshop mode state

## Performance Notes

- Workshop mode optimized for presentation (60 fps)
- Smooth animations on all supported browsers
- 100 recent events stored in memory (older events cleaned up)
- Socket.IO real-time updates for instant feedback
- Checkpoints stored in browser localStorage (max ~5-10 MB)

## Support

For issues or questions:
1. Check browser console (F12)
2. Check server logs
3. Verify all services are running
4. Try npm run install:all to rebuild
