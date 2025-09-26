#!/bin/sh

# Start script for Redis Cache Heatmap container

echo "🚀 Starting Redis Cache Heatmap..."

# Create necessary directories
mkdir -p /var/www/certbot /var/log/nginx

# Start C++ Redis server in background
echo "📊 Starting C++ Redis server..."
./redis-server &
REDIS_PID=$!

# Wait for Redis server to start
sleep 2

# Start Node.js backend in background
echo "🔧 Starting Node.js backend..."
cd server
USE_SCUFFED_REDIS=true PORT=4000 node dist/index.js &
BACKEND_PID=$!
cd ..

# Wait for backend to start
sleep 2

# Start Next.js frontend in background
echo "🌐 Starting Next.js frontend..."
cd web
PORT=3000 HOSTNAME=0.0.0.0 node server.js &
FRONTEND_PID=$!
cd ..

# Wait for frontend to start
sleep 3

# Check if SSL certificates exist
if [ ! -f "/etc/letsencrypt/live/gopee.dev/fullchain.pem" ]; then
    echo "🔒 SSL certificates not found. Obtaining certificates..."
    
    # Start nginx temporarily for HTTP challenge
    nginx -t && nginx
    
    # Get SSL certificate
    certbot --nginx -d gopee.dev --non-interactive --agree-tos --email admin@gopee.dev --redirect
    
    # Stop nginx
    nginx -s quit
    sleep 2
fi

# Start nginx with SSL
echo "🌍 Starting nginx with SSL..."
nginx -t && nginx -g "daemon off;" &
NGINX_PID=$!

# Function to handle shutdown
shutdown() {
    echo "🛑 Shutting down services..."
    kill $NGINX_PID $FRONTEND_PID $BACKEND_PID $REDIS_PID 2>/dev/null
    exit 0
}

# Handle signals
trap shutdown SIGTERM SIGINT

echo "✅ All services started successfully!"
echo "📍 Redis Cache Heatmap available at: https://gopee.dev/scuffedredis"

# Wait for any process to exit
wait
