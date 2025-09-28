FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source files
COPY . .

# Install Node.js dependencies
RUN npm install && \
    npm install --prefix server && \
    npm install --prefix web

EXPOSE 6379 4000 3000

CMD ["make", "run"]
