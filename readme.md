# ScuffedRedis

A Redis implementation built from scratch in C++ with 3D cache visualization.

## What is this?

ScuffedRedis is a complete Redis-compatible server written in C++ that implements:
- Core Redis commands (GET, SET, DEL, EXISTS, KEYS, PING, ECHO, INFO)
- Sorted sets (ZADD, ZRANGE, ZRANK, ZREM, ZSCORE, ZCARD)
- TTL support with automatic expiration
- Custom binary protocol for client-server communication
- Thread-safe hash table with dynamic resizing
- Real-time 3D visualization of cache hits and misses

## Architecture

### C++ Server
- **Port**: 6379 (configurable)
- **Protocol**: Custom binary format for efficiency
- **Data Structures**: Hash table, AVL tree, TTL manager
- **Networking**: TCP server with select/poll event loop
- **Thread Safety**: Concurrent operations with proper locking

### Visualization Stack
- **Backend**: Node.js + Express + Socket.IO (port 4000)
- **Frontend**: Next.js + React Three Fiber (port 3000)
- **Real-time**: WebSocket communication for live updates
- **3D Engine**: 400-voxel wall showing cache activity

## Quick Start

### Prerequisites
- CMake 3.10+
- C++17 compiler
- Node.js 18+
- Docker (optional)

### Build and Run
```bash
# Build C++ server
cmake -B build -S .
cmake --build build --config Release

# Start ScuffedRedis server
./build/Release/scuffed-redis-server

# In another terminal, start visualization
npm install
npm run dev
```

### Using Docker Compose
```bash
docker-compose up --build
```

## Testing

### CLI Client
```bash
# Connect to server
./build/Release/scuffed-redis-cli

# Test commands
> SET hello world
> GET hello
> KEYS *
> INFO
```

### Visualization
Open http://localhost:3000 to see the 3D cache heatmap.

## Redis Commands Supported

### Basic Operations
- `GET key` - Retrieve value
- `SET key value [EX seconds]` - Store with optional TTL
- `DEL key [key ...]` - Delete keys
- `EXISTS key [key ...]` - Check existence
- `KEYS pattern` - Find keys by pattern

### Server Operations
- `PING [message]` - Test connectivity
- `ECHO message` - Echo message back
- `INFO` - Server statistics
- `FLUSHDB` - Clear all data
- `DBSIZE` - Count keys

### Sorted Sets
- `ZADD key score member` - Add to sorted set
- `ZRANGE key start stop` - Get range by rank
- `ZRANK key member` - Get member rank
- `ZREM key member` - Remove member
- `ZSCORE key member` - Get member score
- `ZCARD key` - Get set size

## Implementation Details

### Data Structures
- **Hash Table**: Separate chaining with MurmurHash3
- **AVL Tree**: Self-balancing for sorted sets
- **TTL Manager**: Min-heap for efficient expiration

### Protocol
Binary format: `[Type:1][Length:4][Data:N]`
- Type codes: String(1), Error(2), Integer(3), BulkString(4), Array(5), Null(6)
- Little-endian encoding for multi-byte fields
- Zero-copy operations where possible

### Performance
- O(1) hash table operations (average case)
- O(log n) sorted set operations
- Sub-millisecond response times
- Handles 1000+ concurrent connections

## Project Structure

```
src/
├── server/          # Main server and command handling
├── client/          # CLI client implementation  
├── network/         # TCP sockets and connection management
├── protocol/        # Binary protocol serialization
├── data/            # Hash table, AVL tree, TTL manager
├── event/           # Event loop for async I/O
└── utils/           # Logging utilities

server/              # Node.js visualization backend
web/                 # React frontend with 3D visualization
tests/               # Unit and integration tests
```

## Development Notes

### Building
The project uses CMake for C++ compilation and npm for the Node.js components. The C++ server can run independently or with the visualization stack.

### Protocol Compatibility
ScuffedRedis uses a custom binary protocol rather than Redis RESP. This was designed for educational purposes to understand protocol design. The visualization uses a compatibility layer to communicate with the C++ server.

### Deployment
For production deployment at gopee.dev/scuffedredis:
1. Build optimized binaries with CMAKE_BUILD_TYPE=Release
2. Configure reverse proxy for port 3000 frontend
3. Set up process management (systemd/PM2)
4. Configure firewall for ports 6379 (Redis) and 4000 (API)
5. Use Docker for consistent deployment environment

## License

MIT License - Educational project for learning Redis internals.