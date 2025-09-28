# Deployment Notes for gopee.dev/scuffedredis

## Server Setup

### 1. Build Environment
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake nodejs npm

# Clone repository
git clone https://github.com/ndg8743/ScuffedRedis.git
cd ScuffedRedis
```

### 2. Build Application
```bash
# Install Node.js dependencies
make install

# Build C++ server
make build
```

### 3. Production Configuration

#### Nginx Configuration
```nginx
server {
    listen 80;
    server_name gopee.dev;
    
    location /scuffedredis {
        proxy_pass http://localhost:3000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
    
    location /scuffedredis/api {
        proxy_pass http://localhost:4000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
    }
}
```

#### Systemd Service (scuffedredis.service)
```ini
[Unit]
Description=ScuffedRedis Server
After=network.target

[Service]
Type=simple
User=www-data
WorkingDirectory=/var/www/ScuffedRedis
ExecStart=/var/www/ScuffedRedis/build/Release/scuffed-redis-server 6379
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

#### PM2 Configuration (ecosystem.config.js)
```javascript
module.exports = {
  apps: [
    {
      name: 'scuffedredis-backend',
      script: 'npm',
      args: 'run start --prefix server',
      cwd: '/var/www/ScuffedRedis',
      env: {
        NODE_ENV: 'production',
        PORT: 4000
      }
    },
    {
      name: 'scuffedredis-frontend',
      script: 'npm',
      args: 'run start --prefix web',
      cwd: '/var/www/ScuffedRedis',
      env: {
        NODE_ENV: 'production',
        PORT: 3000
      }
    }
  ]
};
```

### 4. Security Setup

#### Firewall Rules
```bash
# Allow required ports
sudo ufw allow 22      # SSH
sudo ufw allow 80      # HTTP
sudo ufw allow 443     # HTTPS
sudo ufw allow 6379    # ScuffedRedis
sudo ufw allow 4000    # Backend API
sudo ufw allow 3000    # Frontend
```

#### SSL Certificate (Let's Encrypt)
```bash
sudo apt install certbot python3-certbot-nginx
sudo certbot --nginx -d gopee.dev
```

### 5. Deployment Script
```bash
#!/bin/bash
# deploy.sh

set -e

echo "Deploying ScuffedRedis to production..."

# Stop services
sudo systemctl stop scuffedredis
pm2 stop all

# Update code
git pull origin main

# Rebuild
make clean
make build

# Install Node.js dependencies
npm ci --production
npm ci --production --prefix server
npm ci --production --prefix web

# Build frontend
npm run build --prefix web

# Restart services
sudo systemctl start scuffedredis
pm2 start ecosystem.config.js

echo "Deployment complete!"
```

### 6. Monitoring

#### Health Checks
```bash
# Check C++ server
curl -f http://localhost:6379 || echo "ScuffedRedis down"

# Check backend API
curl -f http://localhost:4000/health || echo "Backend down"

# Check frontend
curl -f http://localhost:3000 || echo "Frontend down"
```

#### Log Monitoring
```bash
# C++ server logs
sudo journalctl -u scuffedredis -f

# Node.js logs
pm2 logs scuffedredis-backend
pm2 logs scuffedredis-frontend
```

### 7. Performance Tuning

#### System Limits
```bash
# /etc/security/limits.conf
* soft nofile 65536
* hard nofile 65536
```

#### Kernel Parameters
```bash
# /etc/sysctl.conf
net.core.somaxconn = 65536
net.ipv4.tcp_max_syn_backlog = 65536
```

### 8. Backup Strategy

#### Data Backup
ScuffedRedis stores data in memory only. For persistence:
1. Implement periodic snapshots
2. Use Redis RDB format compatibility
3. Regular filesystem backups of any config files

#### Code Backup
Repository is backed up on GitHub. For deployment:
1. Use tagged releases for production
2. Keep deployment scripts in version control
3. Document configuration changes

### 9. Troubleshooting

#### Common Issues
- Port conflicts: Check with `netstat -tulpn`
- Permission issues: Ensure www-data has access
- Memory limits: Monitor with `htop` or `ps aux`
- Connection limits: Check `ulimit -n`

#### Debug Mode
```bash
# Run with debug logging
CMAKE_BUILD_TYPE=Debug make build
./build/Debug/scuffed-redis-server --log-level=DEBUG
```

### 10. Updates

#### Rolling Updates
1. Build new version in separate directory
2. Test with health checks
3. Stop old services
4. Start new services
5. Verify functionality
6. Clean up old build

#### Rollback Plan
1. Keep previous build directory
2. Stop new services
3. Start previous version
4. Investigate issues
5. Fix and redeploy
