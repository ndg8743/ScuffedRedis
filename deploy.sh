#!/bin/bash

# Redis Cache Heatmap Deployment Script
# Deploy to gopee.dev/scuffedredis

set -e

echo "🚀 Starting deployment to gopee.dev/scuffedredis"

# Check if Docker and Docker Compose are installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker first."
    exit 1
fi

if ! command -v docker-compose &> /dev/null; then
    echo "❌ Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

# Load production environment
if [ -f env.production ]; then
    source env.production
    echo "✅ Loaded production environment"
else
    echo "⚠️  No env.production file found, using defaults"
fi

# Stop existing containers
echo "🛑 Stopping existing containers..."
docker-compose down --remove-orphans

# Remove old images (optional - uncomment to force rebuild)
# echo "🗑️  Removing old images..."
# docker-compose down --rmi all

# Build and start services
echo "🔨 Building and starting services..."
docker-compose up --build -d

# Wait for services to be healthy
echo "⏳ Waiting for services to start..."
sleep 10

# Check service health
echo "🔍 Checking service health..."

# Check C++ Redis server
if docker-compose exec -T redis-server timeout 5 bash -c '</dev/tcp/localhost/6379' 2>/dev/null; then
    echo "✅ C++ Redis server is healthy"
else
    echo "❌ C++ Redis server is not responding"
    exit 1
fi

# Check backend server
if docker-compose exec -T server wget --no-verbose --tries=1 --spider http://localhost:4000/health 2>/dev/null; then
    echo "✅ Backend server is healthy"
else
    echo "❌ Backend server is not responding"
    exit 1
fi

# Check nginx
if curl -f http://localhost/health >/dev/null 2>&1; then
    echo "✅ Nginx proxy is healthy"
else
    echo "❌ Nginx proxy is not responding"
    exit 1
fi

echo ""
echo "🎉 Deployment completed successfully!"
echo ""
echo "📊 Redis Cache Heatmap is now running at:"
echo "   🌐 http://gopee.dev/scuffedredis"
echo "   📡 API: http://gopee.dev/api"
echo ""
echo "📋 Service Status:"
docker-compose ps

echo ""
echo "📝 View logs with:"
echo "   docker-compose logs -f"
echo ""
echo "🛑 Stop services with:"
echo "   docker-compose down"
