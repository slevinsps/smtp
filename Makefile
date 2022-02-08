SERVER_DIR = server
BUILD_DIR = build
AUTOGEN_DIR = server/autogen
REPORT_DIR = report

all: server

server: building_dir 
	make  -C $(SERVER_DIR) --directory=$(BUILD_DIR) --makefile=../Makefile

autogen_files:
	make autogen_files -C $(SERVER_DIR)

check_tidy:
	make check_tidy -C $(SERVER_DIR)

make_report:
	make -C $(REPORT_DIR)

.PHONY: default_run
make default_run:
	./server/build/server -p 1026 -d mail

.PHONY: valgrind_run
make valgrind_run:
	valgrind --tool=memcheck --track-origins=yes  --leak-check=full --show-reachable=yes  ./server/build/server -p 1026 -d mail

.PHONY: test
make test:
	python ./server/tests/test.py

.PHONY: building_dir
building_dir:
	mkdir -p $(SERVER_DIR)/$(BUILD_DIR)

.PHONY: clean
clean:
	make -C $(SERVER_DIR) clean && make -C $(REPORT_DIR) clean