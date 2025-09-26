#!/bin/bash

# Deploy Redis Cache Heatmap to gopee.dev/scuffedredis

set -e

echo "🚀 Deploying Redis Cache Heatmap to gopee.dev/scuffedredis"

# Build Docker image
echo "🔨 Building Docker image..."
docker build -t redis-cache-heatmap .

# Stop existing container if running
echo "🛑 Stopping existing container..."
docker stop redis-cache-heatmap 2>/dev/null || true
docker rm redis-cache-heatmap 2>/dev/null || true

# Run new container
echo "🏃 Starting new container..."
docker run -d \
  --name redis-cache-heatmap \
  --restart unless-stopped \
  -p 80:80 \
  -p 443:443 \
  -v /etc/letsencrypt:/etc/letsencrypt \
  -v /var/lib/letsencrypt:/var/lib/letsencrypt \
  redis-cache-heatmap

echo "✅ Deployment complete!"
echo "📍 Redis Cache Heatmap is now available at: https://gopee.dev/scuffedredis"

# Show container status
docker ps | grep redis-cache-heatmap
