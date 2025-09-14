# ScuffedRedis - Build Your Own Redis

A Redis implementation built from scratch in C++ following the [Build Your Own Redis](https://build-your-own.org/redis) tutorial.

## 🎯 Project Goals

- Learn network programming with sockets
- Implement efficient data structures (hashtables, AVL trees)
- Build an event-driven architecture
- Understand concurrent I/O models
- Create a production-like key-value store

## Implementation Roadmap

### Part 1: Core Redis (Foundation) ✅

- [x] **Setup**: Project structure and build system
- [x] **Socket Programming**: Basic TCP socket creation and binding
- [x] **TCP Server & Client**: Connection handling and basic communication
- [x] **Protocol Design**: Binary protocol for request/response
- [ ] **Concurrent I/O**: Non-blocking I/O with multiplexing (epoll/select)
- [ ] **Event Loop**: Event-driven architecture for handling connections
- [x] **Key-Value Server**: Basic GET/SET/DEL operations

### Part 2: Advanced Features (In Progress)

- [x] **Hashtable**: Efficient hash table with dynamic resizing
- [x] **Serialization**: Optimized data encoding/decoding (via Protocol)
- [x] **Event Loop**: Select-based multiplexing for concurrent I/O
- [x] **TTL**: Key expiration with time-to-live (heap-based)
- [x] **Testing**: Basic test suite for core components
- [ ] **AVL Tree**: Balanced binary tree for sorted operations
- [ ] **Sorted Sets**: ZADD, ZRANGE, ZRANK implementation
- [ ] **Timers**: Connection timeouts and idle detection
- [ ] **Thread Pool**: Multi-threaded request processing

## 🏗️ Architecture

```
ScuffedRedis/
├── src/
│   ├── server/          # Server main and initialization
│   ├── client/          # CLI client implementation
│   ├── network/         # Socket and TCP handling
│   ├── protocol/        # Request/response protocol
│   ├── data/            # Data structures (hashtable, AVL)
│   ├── event/           # Event loop and async I/O
│   └── utils/           # Utilities (logging, timing)
├── tests/               # Unit and integration tests
├── docs/                # Documentation and notes
└── CMakeLists.txt       # Build configuration
```

## 🚀 Quick Start

### Windows
```bash
# Build
build.bat

# Run server (in one terminal)
build\scuffed-redis-server.exe

# Run client (in another terminal)
build\scuffed-redis-cli.exe
```

### Linux/Mac
```bash
# Build
chmod +x build.sh
./build.sh

# Run server (in one terminal)
./build/scuffed-redis-server

# Run client (in another terminal)
./build/scuffed-redis-cli
```

### Test Commands
```
> SET hello world
OK
> GET hello
"world"
> SET counter 100
OK
> GET counter
"100"
> DEL hello
(integer) 1
> EXISTS hello
(integer) 0
> KEYS *
1) "counter"
```

## 🚀 Building and Running

### Prerequisites
- CMake 3.10+
- C++17 compatible compiler
- Windows: Visual Studio or MinGW
- Linux/Mac: GCC or Clang

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run server
./scuffed-redis-server

# Run client (in another terminal)
./scuffed-redis-cli
```

## 🔍 Current Status

**Phase**: Core Redis Implementation ✅
**Features**: Full Redis command support, Protocol, Event Loop, TTL
**Next**: Advanced data structures (AVL trees, Sorted Sets)

### ✅ Completed Features
- **Network Layer**: Cross-platform sockets, TCP server/client
- **Protocol**: Binary protocol with serialization/parsing
- **Data Storage**: Hash table with dynamic resizing
- **Commands**: GET, SET, DEL, EXISTS, KEYS, PING, ECHO, INFO, FLUSHDB, DBSIZE
- **Event Loop**: Select-based multiplexing for concurrent connections
- **TTL Support**: Key expiration with heap-based timer management
- **Testing**: Basic test suite for core components
- **Build System**: CMake with cross-platform support

### 🚧 In Progress
- **Concurrent I/O**: Non-blocking I/O with multiplexing
- **Event Loop Integration**: Full event-driven server architecture

### 📋 TODO
- **AVL Tree**: Balanced binary tree for sorted operations
- **Sorted Sets**: ZADD, ZRANGE, ZRANK implementation
- **Connection Timeouts**: Idle detection and cleanup
- **Thread Pool**: Multi-threaded request processing

---

*This is a learning project implementing Redis core concepts from scratch.*
