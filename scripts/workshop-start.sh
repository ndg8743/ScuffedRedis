#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}     ScuffedRedis Workshop System Startup Check        ${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"

# Check if Redis server binary exists
echo -e "\n${YELLOW}1. Checking Redis Server Binary...${NC}"
if [ -f "build/scuffed-redis-server" ]; then
    echo -e "${GREEN}✓ Redis server binary found${NC}"
else
    echo -e "${RED}✗ Redis server binary not found${NC}"
    echo -e "${YELLOW}  Building Redis server...${NC}"
    mkdir -p build
    cd build
    cmake ..
    make scuffed-redis-server
    cd ..
fi

# Check Node.js dependencies
echo -e "\n${YELLOW}2. Checking Node.js Dependencies...${NC}"
if [ -d "server/node_modules" ]; then
    echo -e "${GREEN}✓ Server dependencies installed${NC}"
else
    echo -e "${YELLOW}  Installing server dependencies...${NC}"
    cd server && npm install && cd ..
fi

if [ -d "web/node_modules" ]; then
    echo -e "${GREEN}✓ Web dependencies installed${NC}"
else
    echo -e "${YELLOW}  Installing web dependencies...${NC}"
    cd web && npm install && cd ..
fi

# Check environment file
echo -e "\n${YELLOW}3. Checking Environment Configuration...${NC}"
if [ -f "server/env" ]; then
    echo -e "${GREEN}✓ Environment file exists${NC}"
    grep "USE_SCUFFED_REDIS=true" server/env > /dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Configured to use ScuffedRedis${NC}"
    else
        echo -e "${YELLOW}  Updating to use ScuffedRedis...${NC}"
        sed -i '' 's/USE_SCUFFED_REDIS=false/USE_SCUFFED_REDIS=true/' server/env
    fi
else
    echo -e "${YELLOW}  Creating environment file...${NC}"
    echo "PORT=4000" > server/env
    echo "REDIS_URL=redis://localhost:6380" >> server/env
    echo "USE_SCUFFED_REDIS=true" >> server/env
    echo "SCUFFED_REDIS_PORT=6380" >> server/env
fi

echo -e "\n${CYAN}═══════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}System check complete!${NC}"
echo -e "\n${YELLOW}Available Commands:${NC}"
echo -e "  ${GREEN}npm run workshop${NC}     - Start all services (Redis, API, Web)"
echo -e "  ${GREEN}npm run workshop:demo${NC} - Start API and Web only"
echo -e "  ${GREEN}npm test:integration${NC}  - Run integration tests"
echo -e "\n${CYAN}Workshop Features Ready:${NC}"
echo -e "  ✓ Command Playground - Interactive Redis terminal"
echo -e "  ✓ Educational Popups - 6 learning modules"
echo -e "  ✓ Workshop Scenarios - 5 guided exercises"
echo -e "  ✓ Control Panel - Traffic & presentation controls"
echo -e "  ✓ 3D Visualization - Real-time cache heatmap"
echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
