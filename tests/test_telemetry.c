/*
 * Telemetry Engine Unit Test Suite
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "telemetry/packet.h"
#include "telemetry/ring_buffer.h"
#include "telemetry/modbus_rtu.h"
#include "telemetry/can_frame.h"

void test_ring_buffer(void) {
    uint8_t storage[64];
    ring_buffer_t rb;
    ring_buffer_init(&rb, storage, sizeof(storage));
    assert(ring_buffer_is_empty(&rb));
    assert(!ring_buffer_is_full(&rb));
    assert(ring_buffer_available(&rb) == 0);

    const char *msg = "TELEMETRY_PACKET_STREAM_TEST";
    size_t len = strlen(msg);
    size_t written = ring_buffer_write(&rb, (const uint8_t*)msg, len);
    assert(written == len);
    assert(ring_buffer_available(&rb) == len);

    uint8_t out[64];
    size_t read_bytes = ring_buffer_read(&rb, out, len);
    assert(read_bytes == len);
    assert(memcmp(msg, out, len) == 0);
    assert(ring_buffer_is_empty(&rb));
    printf("PASS: test_ring_buffer\n");
}

void test_packet_framing(void) {
    telemetry_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.sync_msb = TELEMETRY_SYNC_WORD_MSB;
    pkt.header.sync_lsb = TELEMETRY_SYNC_WORD_LSB;
    pkt.header.pkt_type = PKT_TYPE_ALARM_EVENT;
    pkt.header.station_id = 99;
    pkt.header.sequence_id = 1001;
    pkt.header.payload_len = 16;
    memset(pkt.payload, 0xAA, 16);

    uint8_t wire[256];
    size_t out_len = 0;
    telemetry_status_t st = telemetry_serialize(&pkt, wire, sizeof(wire), &out_len);
    assert(st == ERR_NONE);

    telemetry_packet_t decoded;
    telemetry_status_t dec_st = telemetry_deserialize(wire, out_len, &decoded);
    assert(dec_st == ERR_NONE);
    assert(decoded.header.station_id == 99);
    assert(decoded.header.sequence_id == 1001);
    assert(decoded.header.payload_len == 16);

    // Corrupt one byte to verify CRC detection
    wire[out_len - 1] ^= 0xFF;
    telemetry_status_t corrupt_st = telemetry_deserialize(wire, out_len, &decoded);
    assert(corrupt_st == ERR_CRC_MISMATCH);
    printf("PASS: test_packet_framing\n");
}

void test_modbus(void) {
    uint16_t holding[100];
    uint16_t input[100];
    for (int i = 0; i < 100; ++i) {
        holding[i] = (uint16_t)(i * 10);
        input[i]   = (uint16_t)(500 + i);
    }
    modbus_rtu_slave_t slave;
    modbus_slave_init(&slave, 0x05, holding, 100, input, 100);

    // Modbus Query: Read 2 holding registers starting at address 10
    uint8_t req[8] = { 0x05, 0x03, 0x00, 0x0A, 0x00, 0x02, 0x00, 0x00 };
    uint16_t crc = telemetry_calculate_crc16(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);
    req[7] = (uint8_t)(crc >> 8);

    uint8_t resp[32];
    int resp_len = modbus_process_frame(&slave, req, 8, resp, sizeof(resp));
    assert(resp_len == 9);
    assert(resp[0] == 0x05);
    assert(resp[1] == 0x03);
    assert(resp[2] == 0x04); // 4 bytes of data
    printf("PASS: test_modbus\n");
}

int main(void) {
    printf("Running SCADA Telemetry Unit Tests...\n");
    test_ring_buffer();
    test_packet_framing();
    test_modbus();
    printf("All unit tests passed successfully!\n");
    return 0;
}
