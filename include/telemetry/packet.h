/*
 * Telemetry Packet Protocol Definition - SCADA & RTU Standard
 * Copyright (c) 2004-2008 Yousef Khaled. All rights reserved.
 */

#ifndef TELEMETRY_PACKET_H
#define TELEMETRY_PACKET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TELEMETRY_SYNC_WORD_MSB     0xAA
#define TELEMETRY_SYNC_WORD_LSB     0x55
#define TELEMETRY_MAX_PAYLOAD_SIZE  1024
#define TELEMETRY_HEADER_SIZE       8
#define TELEMETRY_FOOTER_SIZE       4

typedef enum {
    PKT_TYPE_HEARTBEAT    = 0x01,
    PKT_TYPE_SENSOR_BULK  = 0x02,
    PKT_TYPE_ALARM_EVENT  = 0x03,
    PKT_TYPE_PARAM_CONFIG = 0x04,
    PKT_TYPE_ACK          = 0x05,
    PKT_TYPE_NACK         = 0x06
} telemetry_pkt_type_t;

typedef enum {
    ERR_NONE            = 0,
    ERR_INVALID_SYNC    = -1,
    ERR_LENGTH_OVERFLOW = -2,
    ERR_CRC_MISMATCH    = -3,
    ERR_BUFFER_OVERRUN  = -4,
    ERR_NULL_POINTER    = -5
} telemetry_status_t;

#pragma pack(push, 1)
typedef struct {
    uint8_t  sync_msb;
    uint8_t  sync_lsb;
    uint8_t  pkt_type;
    uint8_t  station_id;
    uint16_t sequence_id;
    uint16_t payload_len;
} telemetry_header_t;

typedef struct {
    uint32_t timestamp_sec;
    uint16_t channel_id;
    int16_t  raw_value;
    float    calibrated_value;
    uint16_t status_flags;
} telemetry_sensor_sample_t;

typedef struct {
    telemetry_header_t header;
    uint8_t            payload[TELEMETRY_MAX_PAYLOAD_SIZE];
    uint32_t           crc32;
} telemetry_packet_t;
#pragma pack(pop)

uint32_t telemetry_calculate_crc32(const uint8_t *data, size_t length);
uint16_t telemetry_calculate_crc16(const uint8_t *data, size_t length);
telemetry_status_t telemetry_serialize(const telemetry_packet_t *pkt, uint8_t *dest, size_t dest_cap, size_t *out_len);
telemetry_status_t telemetry_deserialize(const uint8_t *src, size_t src_len, telemetry_packet_t *out_pkt);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_PACKET_H */
