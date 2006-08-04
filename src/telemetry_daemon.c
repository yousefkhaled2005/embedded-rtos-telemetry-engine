/*
 * Telemetry Ingestion Daemon & SCADA Dispatcher
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "telemetry/packet.h"
#include "telemetry/ring_buffer.h"
#include "telemetry/modbus_rtu.h"
#include "telemetry/can_frame.h"

#define TELEMETRY_STREAM_BUFFER_SIZE 65536
#define NUM_HOLDING_REGS 256
#define NUM_INPUT_REGS   512

static uint8_t ring_storage[TELEMETRY_STREAM_BUFFER_SIZE];
static uint16_t holding_memory[NUM_HOLDING_REGS];
static uint16_t input_memory[NUM_INPUT_REGS];

int main(int argc, char **argv) {
    printf("====================================================\n");
    printf("SCADA Telemetry Acquisition Daemon v1.4.0\n");
    printf("Architecture: Embedded RTOS / POSIX C99 Core\n");
    printf("Author: Yousef Khaled (Grad. 2004, Benha University)\n");
    printf("====================================================\n");

    ring_buffer_t rx_queue;
    ring_buffer_init(&rx_queue, ring_storage, TELEMETRY_STREAM_BUFFER_SIZE);

    modbus_rtu_slave_t slave;
    modbus_slave_init(&slave, 0x01, holding_memory, NUM_HOLDING_REGS, input_memory, NUM_INPUT_REGS);

    printf("[Init] Lock-free ring buffer initialized (%d KB capacity)\n", TELEMETRY_STREAM_BUFFER_SIZE / 1024);
    printf("[Init] Modbus RTU slave online at address 0x01 (512 input registers)\n");

    // Populate simulated sensor channels
    for (int i = 0; i < 32; ++i) {
        input_memory[i] = (uint16_t)(1000 + i * 25); // Simulated sensor readings in calibrated units
    }

    // Packet serialization test
    telemetry_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.sync_msb = TELEMETRY_SYNC_WORD_MSB;
    pkt.header.sync_lsb = TELEMETRY_SYNC_WORD_LSB;
    pkt.header.pkt_type = PKT_TYPE_SENSOR_BULK;
    pkt.header.station_id = 42;
    pkt.header.sequence_id = 101;

    telemetry_sensor_sample_t samples[4];
    for (int i = 0; i < 4; ++i) {
        samples[i].timestamp_sec = 1154685000 + i;
        samples[i].channel_id = (uint16_t)(i + 1);
        samples[i].raw_value = (int16_t)(2048 + i * 128);
        samples[i].calibrated_value = 24.5f + (float)i * 1.2f;
        samples[i].status_flags = 0x0001; // Healthy
    }
    memcpy(pkt.payload, samples, sizeof(samples));
    pkt.header.payload_len = sizeof(samples);

    uint8_t tx_wire[2048];
    size_t wire_len = 0;
    telemetry_status_t status = telemetry_serialize(&pkt, tx_wire, sizeof(tx_wire), &wire_len);
    if (status == ERR_NONE) {
        printf("[Telemetry] Serialized sample packet: %zu bytes, CRC32: 0x%08X\n", wire_len, pkt.crc32);
    }

    // Push into ring buffer
    ring_buffer_write(&rx_queue, tx_wire, wire_len);
    printf("[Queue] Pushed wire frame into ring buffer. Free space: %zu bytes\n", ring_buffer_free_space(&rx_queue));

    // Deserialize from ring buffer
    uint8_t rx_wire[2048];
    size_t pulled = ring_buffer_read(&rx_queue, rx_wire, wire_len);
    telemetry_packet_t rx_pkt;
    telemetry_status_t rx_status = telemetry_deserialize(rx_wire, pulled, &rx_pkt);
    if (rx_status == ERR_NONE) {
        printf("[Telemetry] Frame successfully unpacked! Station ID: %d, Seq: %d, Type: 0x%02X\n",
               rx_pkt.header.station_id, rx_pkt.header.sequence_id, rx_pkt.header.pkt_type);
    }

    printf("[Status] Daemon operational. System ready for continuous acquisition.\n");
    return 0;
}
