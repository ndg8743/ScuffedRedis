# Workshop Quick Start Card

## Launch Workshop

```bash
# Full stack (C++, Server, Web)
npm run workshop

# Just Server + Web (requires C++ already running)
npm run workshop:demo
```

Then open: **http://localhost:3000**

## Top Navigation Buttons

| Button | Purpose | Hot Key |
|--------|---------|---------|
| **Command** | Manual Redis commands | Ctrl+K |
| **Learn** | 8 educational modules | Ctrl+L |
| **Scenarios** | 4 guided scenarios | Ctrl+S |
| **Control** | Traffic & checkpoints | Ctrl+Shift+C |

## Control Panel Essentials

### Start Traffic
1. Click **Control** button
2. Set rate (1-100 ops/sec)
3. Choose pattern: constant, spike, wave, random
4. Select operation: read, write, mixed
5. Click **Start**

### Save a Checkpoint
1. Configure desired state
2. Click **Save Current State as Checkpoint**
3. Use **Load** button to restore

### Reset Everything
1. Click **Reset Statistics**
2. Traffic continues (click Stop to end)

## 4 Learning Scenarios

| # | Name | Duration | Level | Focus |
|---|------|----------|-------|-------|
| 1 | Cold vs Warm Cache | 10 min | Beginner | Hit ratio warmup |
| 2 | Traffic Patterns | 15 min | Intermediate | Pattern impact |
| 3 | Operation Mix | 12 min | Intermediate | Read/write balance |
| 4 | Performance Tuning | 23 min | Advanced | Optimization |

## Key Metrics to Watch

- **Hit Ratio**: Green cells in visualization (target: 80%+)
- **Ops/Sec**: Current throughput indicator
- **Latency**: Flash duration (green=1ms, red=100ms+)

## Traffic Patterns at a Glance

```
Constant  ===== (steady, predictable)
Spike     ═╗___ (bursts and lulls)
Wave      ╱╲╱╲╱ (gradual cycles)
Random    ↕↕↕↕↕ (unpredictable)
```

## API Endpoints Summary

```
POST /workshop/traffic         - Control traffic (start/pause/stop)
POST /workshop/reset-stats     - Reset all statistics
POST /workshop/save-state      - Save current state
POST /workshop/load-state      - Restore saved state
GET  /workshop/states          - List all saved states
```

## File Structure

```
/Users/nathan/Downloads/ScuffedRedis/
├── web/
│   ├── components/
│   │   ├── WorkshopControlPanel.tsx    (NEW)
│   │   ├── WorkshopScenarios.tsx       (EXISTING)
│   │   ├── EducationalPopups.tsx       (EXISTING)
│   │   └── TopBar.tsx                  (UPDATED)
│   ├── app/
│   │   └── page.tsx                    (UPDATED)
│   └── lib/
│       ├── state.ts                    (UPDATED)
│       └── socket.ts                   (UPDATED)
├── server/src/
│   ├── index.ts                        (UPDATED +100 lines)
│   └── traffic.ts                      (UPDATED +60 lines)
├── WORKSHOP_GUIDE.md                   (NEW - Main reference)
├── WORKSHOP_INTEGRATION_SUMMARY.md     (NEW - Technical details)
└── WORKSHOP_QUICK_START.md            (THIS FILE)
```

## Common Workflows

### For Beginners
```
1. Start with Constant pattern (10 ops/sec)
2. Watch hit ratio increase
3. Click "Learn" for educational content
4. Run Scenario 1: Cold vs Warm Cache
```

### For Interactive Demo
```
1. Start with Spike pattern (20 ops/sec)
2. Dramatic bursts show cache resilience
3. Use Control Panel to pause and explain
4. Save state before important moments
5. Load state to show comparisons
```

### For Advanced Audience
```
1. Run Scenario 4: Performance Tuning
2. Switch between patterns to show impact
3. Adjust operation type mix
4. Save states as reference points
5. Load saved states for A/B comparison
```

## Troubleshooting Checklist

| Issue | Solution |
|-------|----------|
| Can't start | Ports 3000/4000/6379 free? |
| No traffic | Click Start in Control Panel |
| States not saving | Check browser localStorage enabled |
| Slow performance | Reduce ops/sec or pattern complexity |
| Socket disconnected | Check server running, refresh page |

## Time Allocation (60 minutes)

```
0-5 min   : Intro + show visualization
5-15 min  : Scenario 1 (Cold vs Warm)
15-30 min : Educational content (5-6 topics)
30-45 min : Scenario 2 (Traffic Patterns)
45-55 min : Scenario 3 (Operation Mix)
55-60 min : Q&A + final checkpoint save
```

## Pro Tips

1. **Pre-setup**: Create 2-3 checkpoints before presenting
2. **Pause for explanation**: Use pause button to stop traffic during talking
3. **Save key moments**: Save states showing good hit ratios
4. **Pattern combo**: Combine patterns to show real-world behavior
5. **Multi-monitor**: Control panel on one screen, visualization on another

## Socket.IO Events (Behind the Scenes)

```javascript
cache_event      → Cache hit/miss (includes operationType)
stats_reset      → Stats cleared
state_loaded     → Checkpoint restored
initial_stats    → Connection established
```

## Key Code Locations

| What | Where |
|------|-------|
| UI Buttons | `TopBar.tsx` lines 84-118 |
| Modal Dialogs | `page.tsx` lines 89-111 |
| State Management | `state.ts` lines 47-52, 88-95 |
| API Endpoints | `index.ts` lines 158-252 |
| Traffic Patterns | `traffic.ts` lines 66-87 |
| Socket Events | `socket.ts` lines 34-54 |

## Documentation Map

- **For Using Workshop**: Read `WORKSHOP_GUIDE.md` (335+ lines, comprehensive)
- **For Technical Details**: Read `WORKSHOP_INTEGRATION_SUMMARY.md` (413+ lines)
- **For Quick Reference**: You are here (this file)

## Next Steps After Presentation

1. **Save Results**: Checkpoints auto-saved to localStorage
2. **Export Stats**: Use browser DevTools to export state
3. **Feedback**: Note which scenarios were most engaging
4. **Iteration**: Use saved states to refine future presentations

## Emergency Shortcuts

```bash
# Full restart
npm run install:all && npm run workshop:demo

# Just frontend
npm run dev:web --prefix web

# Just backend
npm run dev:server --prefix server

# Clear everything and start fresh
rm -rf server/node_modules web/node_modules
npm run install:all && npm run workshop
```

---

**Ready to present!** Start with `npm run workshop:demo` and open http://localhost:3000
