# 🚀 ScuffedRedis Workshop - Complete Guide

## Overview
A comprehensive 1-hour workshop demonstrating Redis caching concepts through an interactive 3D visualization and hands-on exercises.

## Quick Start

```bash
# Run startup check
./workshop-start.sh

# Start everything
npm run workshop

# Or start without Redis server (demo mode)
npm run workshop:demo
```

Then open: http://localhost:3000

## Workshop Components

### 1. **3D Cache Heatmap Visualization**
- **Green flashes**: Cache hits (fast, <10ms)
- **Red flashes**: Cache misses (slow, 600-1200ms)
- **Blue flashes**: SET operations
- **Orange flashes**: Expirations
- Real-time operations counter
- Hit ratio display with color coding

### 2. **Command Playground**
Access via Terminal button in top bar
- Interactive Redis terminal
- Syntax-highlighted commands
- Command history (↑/↓ arrows)
- Example commands sidebar
- Execution timing display
- Support for all 16 Redis commands

### 3. **Educational Popups**
Access via Learn button
- 6 interactive learning modules:
  1. How Redis Caching Works
  2. Hash Table Implementation
  3. Binary Protocol
  4. AVL Trees for Sorted Sets
  5. TTL & Expiration
  6. Performance Metrics

### 4. **Workshop Scenarios**
Access via Scenarios button
- **Cold vs Warm Cache** (10 min, Beginner)
- **TTL and Expiration** (15 min, Intermediate)
- **Building a Leaderboard** (20 min, Advanced)
- **Pattern Matching & Search** (15 min, Intermediate)
- **Preventing Cache Stampede** (20 min, Advanced)

### 5. **Control Panel**
Access via Control button
- Traffic rate control (1-100 ops/sec)
- Traffic patterns: constant, spike, wave, random
- Operation types: read-heavy, write-heavy, mixed
- Save/load checkpoints
- Reset statistics
- Performance metrics display

## Supported Redis Commands

### Basic Operations
- `PING` - Test connectivity
- `SET key value [EX seconds]` - Store with optional TTL
- `GET key` - Retrieve value
- `DEL key` - Delete key
- `EXISTS key` - Check existence

### Server Commands
- `INFO` - Server statistics
- `DBSIZE` - Count total keys
- `FLUSHDB` - Clear database
- `KEYS pattern` - Pattern matching

### Sorted Sets
- `ZADD key score member` - Add to sorted set
- `ZRANGE key start stop [WITHSCORES]` - Get range
- `ZRANK key member` - Get rank
- `ZREM key member` - Remove member
- `ZSCORE key member` - Get score
- `ZCARD key` - Get cardinality

## Workshop Timeline (60 minutes)

### Introduction (5 min)
1. Open visualization at http://localhost:3000
2. Explain the color coding
3. Show initial cold cache behavior

### Part 1: Cache Basics (15 min)
1. Open **Command Playground**
2. Execute basic SET/GET commands
3. Click **Warm Up** button
4. Observe hit ratio improvement
5. Run "Cold vs Warm Cache" scenario

### Part 2: Advanced Features (20 min)
1. Open **Educational Popups**
2. Review Hash Table implementation
3. Demonstrate TTL with expiring keys
4. Build a leaderboard with sorted sets
5. Show pattern matching with KEYS

### Part 3: Performance & Optimization (15 min)
1. Open **Control Panel**
2. Adjust traffic patterns
3. Compare operation types
4. Save checkpoint before scenario switch
5. Demonstrate cache stampede prevention

### Q&A and Exploration (5 min)
- Let participants try commands
- Explore different scenarios
- Discuss real-world applications

## Testing

Run integration tests to verify everything works:

```bash
npm run test:integration
```

This will test:
- Redis server connectivity
- API endpoints
- Workshop features
- Command execution
- State management

## Architecture

```
┌─────────────────┐
│  Browser (3000) │  ← 3D Visualization + UI
└────────┬────────┘
         │ WebSocket
         ↓
┌─────────────────┐
│ Node.js (4000)  │  ← API + Traffic Generator
└────────┬────────┘
         │ Binary Protocol
         ↓
┌─────────────────┐
│ C++ Redis (6380)│  ← Full Redis Implementation
└─────────────────┘
```

## Tips for Presenters

1. **Start Simple**: Begin with basic GET/SET before advanced features
2. **Use Scenarios**: Follow the guided scenarios for structured learning
3. **Interactive Demo**: Let participants suggest commands to run
4. **Save States**: Create checkpoints before major demonstrations
5. **Monitor Performance**: Keep Control Panel visible for metrics

## Troubleshooting

### Redis server not connecting
```bash
# Check if running
ps aux | grep scuffed-redis-server

# Restart
pkill scuffed-redis-server
npm run dev:cpp
```

### Node.js API issues
```bash
# Check logs
npm run dev:server

# Verify environment
cat server/env
```

### Frontend not updating
```bash
# Clear cache and restart
rm -rf web/.next
npm run dev:web
```

## Key Learning Outcomes

Participants will understand:
- ✅ Cache-aside pattern and its benefits
- ✅ Impact of cache hit ratio on performance
- ✅ Redis data structures (strings, sorted sets)
- ✅ TTL and automatic expiration
- ✅ Pattern matching and key organization
- ✅ Real-world caching strategies
- ✅ Performance optimization techniques

## Resources

- Command reference: Use Command Playground examples
- Architecture details: See Educational Popups
- Practice exercises: Follow Workshop Scenarios
- Performance tuning: Use Control Panel

## Support

For issues or questions about the workshop:
1. Run `./workshop-start.sh` to verify setup
2. Check `test-integration.js` for system tests
3. Review server logs in console output

---

**Ready to start?** Run `npm run workshop` and open http://localhost:3000

Enjoy your Redis workshop! 🎉