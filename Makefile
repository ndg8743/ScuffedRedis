# ScuffedRedis Makefile

# Detect OS
ifeq ($(OS),Windows_NT)
    CMAKE = cmake
    BUILD_CMD = cmake --build . --config Release
    SERVER_EXE = Release/scuffed-redis-server.exe
    CLIENT_EXE = Release/scuffed-redis-cli.exe
else
    CMAKE = cmake
    BUILD_CMD = make -j$(shell nproc)
    SERVER_EXE = ./scuffed-redis-server
    CLIENT_EXE = ./scuffed-redis-cli
endif

BUILD_DIR = build

.PHONY: all build clean run install

all: build

build:
	@echo "Building ScuffedRedis..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_BUILD_TYPE=Release
	@cd $(BUILD_DIR) && $(BUILD_CMD)
	@echo "Build complete!"

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

run: build
	@echo "Starting ScuffedRedis server..."
	@cd $(BUILD_DIR) && $(SERVER_EXE) &
	@sleep 2
	@echo "Starting visualization backend..."
	@npm run dev --prefix server &
	@sleep 3
	@echo "Starting frontend..."
	@npm run dev --prefix web

install:
	@npm install
	@npm install --prefix server
	@npm install --prefix web

test: build
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
