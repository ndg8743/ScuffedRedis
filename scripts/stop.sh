#!/bin/bash

# Stop all ScuffedRedis services

echo "Stopping all ScuffedRedis services..."

# Kill processes on specific ports
lsof -ti :6380 :4000 :3000 | xargs kill -9 2>/dev/null

# Kill processes by name
pkill -f "scuffed-redis-server" 2>/dev/null
pkill -f "ts-node-dev.*server" 2>/dev/null
pkill -f "next dev" 2>/dev/null

sleep 1

echo "All services stopped."

