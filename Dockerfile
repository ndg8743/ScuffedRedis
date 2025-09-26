# Multi-stage build for Redis Cache Heatmap
FROM ubuntu:22.04 AS cpp-builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY simple_server.cpp CMakeLists.txt ./
RUN mkdir build && cd build && cmake .. && make

# Node.js backend stage
FROM node:18-alpine AS server-builder

WORKDIR /app
COPY server/package*.json ./
RUN npm ci --only=production

COPY server/src ./src
COPY server/tsconfig.json ./
RUN npm install -g typescript && tsc

# Next.js frontend stage  
FROM node:18-alpine AS web-builder

WORKDIR /app
COPY web/package*.json ./
RUN npm ci

COPY web/ ./
RUN npm run build

# Production runtime
FROM node:18-alpine AS runtime

# Install nginx and certbot
RUN apk add --no-cache nginx certbot certbot-nginx

# Copy built applications
WORKDIR /app

# Copy C++ Redis server
COPY --from=cpp-builder /app/build/redis-server ./redis-server

# Copy Node.js backend
COPY --from=server-builder /app/dist ./server/dist
COPY --from=server-builder /app/node_modules ./server/node_modules

# Copy Next.js frontend
COPY --from=web-builder /app/.next/standalone ./web/
COPY --from=web-builder /app/.next/static ./web/.next/static
COPY --from=web-builder /app/public ./web/public

# Copy nginx config and startup script
COPY nginx.conf /etc/nginx/nginx.conf
COPY start.sh ./
RUN chmod +x start.sh

# Create nginx user and directories
RUN adduser -D -s /bin/sh nginx || true
RUN mkdir -p /var/log/nginx /var/lib/nginx/tmp /run/nginx
RUN chown -R nginx:nginx /var/log/nginx /var/lib/nginx /run/nginx

EXPOSE 80 443

CMD ["./start.sh"]
