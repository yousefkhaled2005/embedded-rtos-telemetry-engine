CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Iinclude -std=c99
SRC = src/crc_engine.c src/ring_buffer.c src/packet_parser.c src/modbus_rtu.c src/can_filter.c

all: bin/telemetry_daemon bin/test_telemetry

bin/telemetry_daemon: $(SRC) src/telemetry_daemon.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

bin/test_telemetry: $(SRC) tests/test_telemetry.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

test: bin/test_telemetry
	./bin/test_telemetry

clean:
	rm -rf bin/
