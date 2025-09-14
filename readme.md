# ScuffedRedis

A Redis-like in-memory key-value store implementation in C++, built from scratch following clean code principles.

## Features

- **Network Programming**: TCP server/client with concurrent I/O
- **Data Structures**: Efficient hashtables, balanced binary trees, sorted sets
- **Event Loop**: High-performance event-driven architecture
- **Caching**: TTL-based cache expiration with timer management
- **Thread Pool**: Concurrent request processing

## Architecture

```
src/
├── network/          # Socket programming and TCP handling
├── protocol/         # Request-response protocol implementation
├── data/            # Data structures (hashtables, trees, sets)
├── server/          # Main server logic and event loop
├── client/          # Client implementation
└── utils/           # Utilities and helpers
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
# Start server
./scuffedredis-server --port 6379

# Connect with client
./scuffedredis-client --host localhost --port 6379
```

## Design Principles

- **Clean Code**: Readable, maintainable code with clear comments
- **Performance**: Optimized for speed and efficiency
- **Caching**: Intelligent caching strategies throughout
- **Modularity**: Well-separated concerns and components

Based on the excellent tutorial at [build-your-own.org/redis](https://build-your-own.org/redis/)
