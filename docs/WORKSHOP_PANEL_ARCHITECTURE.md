# Workshop Control Panel Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                      React Frontend (Next.js)                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌──────────────────────────┐         ┌──────────────────────────┐  │
│  │  Main Visualization      │         │ Workshop Control Panel   │  │
│  │  (Heatmap/3D Scene)      │         │ (Floating Component)     │  │
│  │                          │         │                          │  │
│  │  - Cache visualization   │         │ - Presentation Mode      │  │
│  │  - Real-time updates     │         │ - Traffic Control        │  │
│  │  - User interactions     │         │ - Live Metrics           │  │
│  └──────────────────────────┘         │ - Quick Actions          │  │
│                                       │ - Scenario Launcher      │  │
│                                       └──────────────────────────┘  │
│                                                                       │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │              Zustand Store (useAppStore)                     │   │
│  │                                                               │   │
│  │  hitRatio: { hits, misses, ratio }                          │   │
│  │  events: CacheEvent[]                                       │   │
│  │  isConnected: boolean                                       │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                                                                       │
└─────────────────────────────────────────────────────────────────────┘
          │                             │                  │
          │ Fetch API                   │ Fetch API        │ Real-time
          │                             │                  │ Updates
          ▼                             ▼                  ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        Backend Server (Node/Express)               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌──────────┐  │
│  │Traffic      │  │Metrics      │  │Actions      │  │Scenarios │  │
│  │Generator    │  │Collector    │  │Handler      │  │Manager   │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  └──────────┘  │
│       │                  │                │               │         │
│       └──────────────────┴────────────────┴───────────────┘         │
│                          │                                           │
│                   ┌──────▼──────┐                                    │
│                   │ Redis Cache  │                                   │
│                   └──────────────┘                                   │
│                                                                       │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Data Flow

```
WorkshopControlPanel
│
├── State Management
│   ├── presentationState (local)
│   │   ├── isRunning
│   │   ├── isPaused
│   │   ├── currentScenario
│   │   ├── currentStep
│   │   └── elapsedSeconds
│   │
│   ├── trafficConfig (local)
│   │   ├── rate
│   │   ├── pattern
│   │   └── isPaused
│   │
│   ├── metrics (local, rolling)
│   │   └── MetricsSnapshot[] (max 60)
│   │
│   └── expandedSections (local UI state)
│       ├── presentation
│       ├── traffic
│       ├── actions
│       ├── metrics
│       └── scenarios
│
├── External State
│   ├── useAppStore.hitRatio (Zustand)
│   └── SERVER_URL (config)
│
├── Effects
│   ├── Timer (1s interval, presentation running)
│   └── Metrics Polling (1s interval, always)
│
└── API Calls
    ├── POST /traffic/rate
    ├── POST /traffic/pattern
    ├── POST /traffic/pause
    ├── POST /traffic/resume
    ├── POST /test-data
    ├── POST /clear
    ├── POST /reset-stats
    └── GET  /metrics
```

## Presentation Mode State Machine

```
            ┌─────────────────────────────────────────────┐
            │       STOPPED (Initial State)                │
            │                                              │
            │  isRunning: false                            │
            │  isPaused: false                             │
            │  elapsedSeconds: 0                           │
            └──────────────────┬──────────────────────────┘
                               │
                    Click "Start" button
                               │
                               ▼
            ┌─────────────────────────────────────────────┐
            │       RUNNING                                │
            │                                              │
            │  isRunning: true                             │
            │  isPaused: false                             │
            │  Timer increments every 1s                   │
            └──────────────────┬──────────────────────────┘
                               │
                ┌──────────────┼──────────────┐
                │                             │
        Click "Pause"              Click "Stop"
                │                             │
                ▼                             ▼
     ┌────────────────────┐     ┌─────────────────────┐
     │    PAUSED          │     │  STOPPED            │
     │                    │     │                     │
     │ isPaused: true     │     │ Reset all state     │
     │ Timer frozen       │     │ Go to STOPPED       │
     │                    │     └─────────────────────┘
     │ Click "Resume"     │
     │      or            │
     │ "Pause" again ─────┘
```

## Traffic Generation Control

```
Traffic Generation Process

┌──────────────────────────────────────────┐
│   Traffic Configuration Updated          │
│   (via component or API)                 │
└──────────────┬───────────────────────────┘
               │
               ├─ rate changed?     ──→ POST /traffic/rate
               │
               ├─ pattern changed?  ──→ POST /traffic/pattern
               │
               └─ pause toggled?
                  ├─ true   ──→ POST /traffic/pause
                  └─ false  ──→ POST /traffic/resume

Server-side generation:

┌───────────────────────────┐
│  Rate: ops/sec            │
│  Pattern: uniform/zipf/   │
│           burst           │
└───────────┬───────────────┘
            │
            ├─ Generate operations
            │  at configured rate
            │
            ├─ Apply pattern
            │  distribution
            │
            └─ Execute against
               Redis cache
```

## Metrics Collection Pipeline

```
┌─────────────────────────────────────────────────────────────┐
│  Every 1 Second (useEffect with 1s interval)               │
└─────────────────────────┬───────────────────────────────────┘
                          │
                    GET /metrics
                          │
                          ▼
        ┌─────────────────────────────────┐
        │ Server collects from Redis:     │
        │ - ops/sec                       │
        │ - memory usage                  │
        │ - active connections            │
        │ - command distribution          │
        └────────────┬────────────────────┘
                     │
                     ▼
        ┌─────────────────────────────────┐
        │ Response: MetricsSnapshot       │
        │ {                               │
        │   opsPerSecond: number          │
        │   memoryUsageMB: number         │
        │   activeConnections: number     │
        │   commandDistribution: {}       │
        │   timestamp: number             │
        │ }                               │
        └────────────┬────────────────────┘
                     │
                     ▼
        ┌─────────────────────────────────┐
        │ Store in rolling array:         │
        │ metrics = [                     │
        │   ...metrics.slice(-59),        │
        │   newSnapshot                   │
        │ ]                               │
        │ (Max 60 snapshots = 60s history)│
        └────────────┬────────────────────┘
                     │
                     ▼
        ┌─────────────────────────────────┐
        │ Render latestMetric in UI       │
        │ - Display ops/sec               │
        │ - Display memory                │
        │ - Display connections           │
        │ - Sort & display top 5 commands │
        └─────────────────────────────────┘
```

## Save/Load State Workflow

```
SAVE STATE:

Click "Save State"
        │
        ▼
┌──────────────────────────────────┐
│ Construct WorkshopState          │
│ {                                │
│   presentationState: {...},      │
│   trafficConfig: {...},          │
│   metrics: [...]                 │
│ }                                │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Convert to JSON string           │
│ const dataStr = JSON.stringify() │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Create Blob from JSON            │
│ const dataBlob = new Blob()      │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Create download link and trigger │
│ const url = URL.createObjectURL()│
│ link.download = filename         │
│ link.click()                     │
└──────────┬───────────────────────┘
           │
           ▼
    File Downloaded:
    workshop-state-{timestamp}.json


LOAD STATE:

Click "Load State"
        │
        ▼
┌──────────────────────────────────┐
│ File picker opens                │
│ User selects .json file          │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ FileReader reads file            │
│ const reader = new FileReader()  │
│ reader.readAsText(file)          │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Parse JSON string                │
│ const state = JSON.parse()       │
└──────────┬───────────────────────┘
           │
           ▼
┌──────────────────────────────────┐
│ Restore all state:               │
│ setPresentationState(...)        │
│ setTrafficConfig(...)            │
│ setMetrics(...)                  │
└──────────────────────────────────┘
```

## Component Hierarchy

```
App (Next.js Page)
│
├── MainVisualization
│   └── (3D scene, heatmap, etc.)
│
└── WorkshopControlPanel
    │
    ├── Draggable Container (fixed position)
    │   │
    │   └── Card (shadcn/ui)
    │       │
    │       ├── Header (draggable)
    │       │   ├── GripVertical (icon)
    │       │   ├── Title
    │       │   └── Live Badge
    │       │
    │       └── CardContent (scrollable)
    │           │
    │           ├── PresentationSection
    │           │   ├── Play/Pause/Stop Buttons
    │           │   ├── Timer Display
    │           │   ├── Current Scenario Info
    │           │   └── Next Step Button
    │           │
    │           ├── TrafficSection
    │           │   ├── Rate Slider
    │           │   ├── Pattern Buttons
    │           │   └── Pause/Resume Toggle
    │           │
    │           ├── QuickActionsSection
    │           │   ├── Populate Test Data
    │           │   ├── Clear Cache
    │           │   ├── Reset Stats
    │           │   ├── Save State
    │           │   └── Load State
    │           │
    │           ├── MetricsSection
    │           │   ├── Ops/sec Display
    │           │   ├── Memory Usage Display
    │           │   ├── Connections Display
    │           │   ├── Hit Ratio Display
    │           │   └── Top Commands List
    │           │
    │           └── ScenarioSection
    │               ├── Cold vs Warm Button
    │               ├── TTL & Expiration Button
    │               ├── Data Types Button
    │               └── Eviction Policies Button
```

## Drag Implementation

```
Mouse Interaction for Dragging:

Panel Container (with ref)
│
├── onMouseDown on Header [data-drag-handle]
│   │
│   ├─ Check if target has [data-drag-handle]
│   │
│   ├─ setIsDragging(true)
│   │
│   ├─ Calculate drag offset:
│   │  dragOffset.x = clientX - position.x
│   │  dragOffset.y = clientY - position.y
│   │
│   └─ Set cursor to 'grabbing'
│
├── onMouseMove anywhere on Container
│   │
│   └─ If isDragging:
│      ├─ Calculate new position:
│      │  newX = clientX - dragOffset.x
│      │  newY = clientY - dragOffset.y
│      │
│      └─ setPosition({ x: newX, y: newY })
│         └─ Updates inline style:
│            position: fixed;
│            left: newX px;
│            top: newY px;
│
└── onMouseUp or onMouseLeave
    │
    ├─ setIsDragging(false)
    │
    └─ Cursor back to 'grab'
```

## API Contract

```
Traffic Control:

POST /traffic/rate
  Request:  { rate: number }           [1-100]
  Response: { success: true }

POST /traffic/pattern
  Request:  { pattern: string }        ['uniform'|'zipf'|'burst']
  Response: { success: true }

POST /traffic/pause
  Response: { success: true }

POST /traffic/resume
  Response: { success: true }


Quick Actions:

POST /test-data
  Request:  { count: number }
  Response: { success: true, inserted: number }

POST /clear
  Response: { success: true }

POST /reset-stats
  Response: { success: true }


Metrics:

GET /metrics
  Response: {
    opsPerSecond: number,
    memoryUsageMB: number,
    activeConnections: number,
    commandDistribution: {
      [commandName: string]: number
    }
  }
```

## Error Handling Strategy

```
API Call Failure:
  │
  ├─ Fetch error caught
  │  └─ console.error() logged
  │     └─ State updated optimistically (not reverted)
  │
  └─ No UI error message shown
     (Silent failure, relies on browser console)


File Operations:
  │
  ├─ Save State
  │  └─ Always succeeds (local file download)
  │
  └─ Load State
     ├─ File picker cancel
     │  └─ No error (user cancelled)
     │
     ├─ Invalid JSON
     │  └─ catch block logs error
     │     └─ State not updated
     │
     └─ Valid JSON, wrong structure
        └─ Partial state update (what's available)
```

## Performance Characteristics

```
Rendering:
  - Components: ~10-15 total (buttons, cards, sections)
  - Re-renders: On state changes only
  - Optimization: No useMemo/useCallback (component is small)

Memory:
  - Metrics array: 60 items max = ~5-10KB
  - Component state: ~2KB
  - Total: ~15KB runtime

Network:
  - Metrics polling: 1 request/sec (4-5KB response)
  - Traffic updates: On slider/button change (100 bytes)
  - Typical: 30-50 requests/min during presentation

CPU:
  - Timer effect: 1 state update/sec
  - Polling effect: 1 fetch + state update/sec
  - Render time: <16ms (60fps target)
```

## Accessibility Tree

```
WorkshopControlPanel
├── div[fixed position]
│   └── Card
│       ├── div[draggable header]
│       │   ├── svg[grip icon, decorative]
│       │   ├── h2[Workshop Control]
│       │   └── Badge[Live status]
│       │
│       └── CardContent[scrollable]
│           ├── div[Presentation section]
│           │   ├── button[Collapse/Expand]
│           │   ├── button[Start]
│           │   ├── button[Pause/Resume]
│           │   ├── button[Stop]
│           │   └── button[Next Step]
│           │
│           ├── div[Traffic section]
│           │   ├── input[range slider, rate]
│           │   ├── button[uniform pattern]
│           │   ├── button[zipf pattern]
│           │   ├── button[burst pattern]
│           │   └── button[Pause/Resume Traffic]
│           │
│           └── [... more sections ...]
```

---

## Key Design Decisions

1. **No Props**: Component is self-contained for maximum reusability
2. **Floating Panel**: Draggable design for multi-monitor setups
3. **Rolling Metrics**: 60-second window prevents memory bloat
4. **Optimistic Updates**: State updates immediately, API async
5. **Local State**: Only presentation/traffic config, metrics history local
6. **Poll Strategy**: Constant polling for real-time metrics
7. **JSON Persistence**: Browser file I/O for state save/load
8. **Collapsible Sections**: Manage screen real estate for long presentations
