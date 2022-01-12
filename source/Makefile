SERVER_DIR = server
BUILD_DIR = build

all: server

server: building_dir 
	cd $(SERVER_DIR) && make --directory=$(BUILD_DIR) --makefile=../Makefile

test_client: building_dir
	cd $(SERVER_DIR) && make test_client --directory=$(BUILD_DIR) --makefile=../Makefile



.PHONY: building_dir
building_dir:
	mkdir -p $(SERVER_DIR)/$(BUILD_DIR)

.PHONY: clean
clean:
	cd $(SERVER_DIR) && make clean