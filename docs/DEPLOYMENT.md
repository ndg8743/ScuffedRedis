# Deployment Guide

This guide covers deploying the Redis Cache Heatmap to gopee.dev/scuffedredis using Docker Compose.

## Prerequisites

- Docker and Docker Compose installed
- Domain pointing to your server (gopee.dev)
- SSL certificates (optional, for HTTPS)

## Quick Deployment

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd ScuffedRedis
   ```

2. **Deploy with one command:**
   ```bash
   ./deploy.sh
   ```

3. **Access the application:**
   - Frontend: http://gopee.dev/scuffedredis
   - API: http://gopee.dev/api
   - Health check: http://gopee.dev/health

## Manual Deployment

If you prefer manual steps:

```bash
# Build and start all services
docker-compose up --build -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f

# Stop services
docker-compose down
```

## Architecture

The deployment consists of 4 services:

1. **redis-server** - C++ Redis implementation (port 6379)
2. **server** - Node.js backend API (port 4000)
3. **web** - React frontend (port 3000)
4. **nginx** - Reverse proxy (ports 80/443)

## Configuration

### Environment Variables

Edit `env.production`:

```env
# Backend
PORT=4000
USE_SCUFFED_REDIS=true
NODE_ENV=production

# Frontend  
NEXT_PUBLIC_SERVER_URL=http://gopee.dev/api
NODE_ENV=production

# Domain
DOMAIN=gopee.dev
```

### Nginx Configuration

The `nginx.conf` file handles:
- Reverse proxy to frontend and backend
- WebSocket support for real-time updates
- Static asset caching
- Security headers
- Rate limiting

### SSL/HTTPS Setup

To enable HTTPS:

1. **Place SSL certificates in `ssl/` directory:**
   ```
   ssl/
   ├── cert.pem
   └── key.pem
   ```

2. **Update nginx.conf to include HTTPS server block:**
   ```nginx
   server {
       listen 443 ssl;
       server_name gopee.dev;
       
       ssl_certificate /etc/nginx/ssl/cert.pem;
       ssl_certificate_key /etc/nginx/ssl/key.pem;
       
       # ... rest of configuration
   }
   ```

3. **Restart nginx:**
   ```bash
   docker-compose restart nginx
   ```

## Monitoring

### Health Checks

All services have health checks configured:

```bash
# Check all service health
docker-compose ps

# Individual service logs
docker-compose logs redis-server
docker-compose logs server
docker-compose logs web
docker-compose logs nginx
```

### Performance Monitoring

Monitor service resource usage:

```bash
# Container stats
docker stats

# Service-specific stats
docker stats redis-server redis-backend redis-frontend redis-nginx
```

## Troubleshooting

### Common Issues

1. **Port conflicts:**
   ```bash
   # Check what's using port 80
   sudo netstat -tulpn | grep :80
   ```

2. **Service not starting:**
   ```bash
   # Check logs for specific service
   docker-compose logs <service-name>
   ```

3. **Network connectivity:**
   ```bash
   # Test internal network
   docker-compose exec server ping redis-server
   docker-compose exec web ping server
   ```

### Debugging

1. **Access running containers:**
   ```bash
   docker-compose exec redis-server /bin/bash
   docker-compose exec server /bin/sh
   docker-compose exec web /bin/sh
   ```

2. **Test C++ Redis server directly:**
   ```bash
   # From host
   telnet localhost 6379
   
   # From container
   docker-compose exec redis-server telnet localhost 6379
   ```

3. **Check API endpoints:**
   ```bash
   curl http://localhost/health
   curl http://localhost/api/hitratio
   ```

## Scaling

To scale the application:

1. **Scale backend services:**
   ```bash
   docker-compose up --scale server=3 -d
   ```

2. **Update nginx upstream configuration** to include multiple backend instances.

3. **Add load balancing** for the C++ Redis server if needed.

## Updates

To update the application:

```bash
# Pull latest changes
git pull

# Rebuild and restart
docker-compose up --build -d

# Or use deployment script
./deploy.sh
```

## Backup

Backup important data:

```bash
# Export container data
docker-compose exec redis-server <backup-command>

# Backup configuration
tar -czf backup.tar.gz nginx.conf docker-compose.yml env.production
```
