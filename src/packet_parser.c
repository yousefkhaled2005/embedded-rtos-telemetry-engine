/*
 * Telemetry Packet Parser and Framing State Machine
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include "telemetry/packet.h"
#include <string.h>

telemetry_status_t telemetry_serialize(const telemetry_packet_t *pkt, uint8_t *dest, size_t dest_cap, size_t *out_len) {
    if (!pkt || !dest || !out_len) return ERR_NULL_POINTER;
    size_t total_size = TELEMETRY_HEADER_SIZE + pkt->header.payload_len + TELEMETRY_FOOTER_SIZE;
    if (dest_cap < total_size) return ERR_BUFFER_OVERRUN;
    if (pkt->header.payload_len > TELEMETRY_MAX_PAYLOAD_SIZE) return ERR_LENGTH_OVERFLOW;

    // Header serialization
    dest[0] = pkt->header.sync_msb;
    dest[1] = pkt->header.sync_lsb;
    dest[2] = pkt->header.pkt_type;
    dest[3] = pkt->header.station_id;
    dest[4] = (uint8_t)(pkt->header.sequence_id >> 8);
    dest[5] = (uint8_t)(pkt->header.sequence_id & 0xFF);
    dest[6] = (uint8_t)(pkt->header.payload_len >> 8);
    dest[7] = (uint8_t)(pkt->header.payload_len & 0xFF);

    // Payload serialization
    if (pkt->header.payload_len > 0) {
        memcpy(&dest[TELEMETRY_HEADER_SIZE], pkt->payload, pkt->header.payload_len);
    }

    // CRC32 calculation over header + payload
    uint32_t crc = telemetry_calculate_crc32(dest, TELEMETRY_HEADER_SIZE + pkt->header.payload_len);
    size_t footer_idx = TELEMETRY_HEADER_SIZE + pkt->header.payload_len;
    dest[footer_idx]     = (uint8_t)((crc >> 24) & 0xFF);
    dest[footer_idx + 1] = (uint8_t)((crc >> 16) & 0xFF);
    dest[footer_idx + 2] = (uint8_t)((crc >> 8) & 0xFF);
    dest[footer_idx + 3] = (uint8_t)(crc & 0xFF);

    *out_len = total_size;
    return ERR_NONE;
}

telemetry_status_t telemetry_deserialize(const uint8_t *src, size_t src_len, telemetry_packet_t *out_pkt) {
    if (!src || !out_pkt) return ERR_NULL_POINTER;
    if (src_len < TELEMETRY_HEADER_SIZE + TELEMETRY_FOOTER_SIZE) return ERR_BUFFER_OVERRUN;

    if (src[0] != TELEMETRY_SYNC_WORD_MSB || src[1] != TELEMETRY_SYNC_WORD_LSB) {
        return ERR_INVALID_SYNC;
    }

    out_pkt->header.sync_msb    = src[0];
    out_pkt->header.sync_lsb    = src[1];
    out_pkt->header.pkt_type    = src[2];
    out_pkt->header.station_id  = src[3];
    out_pkt->header.sequence_id = (uint16_t)((src[4] << 8) | src[5]);
    out_pkt->header.payload_len = (uint16_t)((src[6] << 8) | src[7]);

    if (out_pkt->header.payload_len > TELEMETRY_MAX_PAYLOAD_SIZE) {
        return ERR_LENGTH_OVERFLOW;
    }

    size_t expected_total = TELEMETRY_HEADER_SIZE + out_pkt->header.payload_len + TELEMETRY_FOOTER_SIZE;
    if (src_len < expected_total) {
        return ERR_BUFFER_OVERRUN;
    }

    if (out_pkt->header.payload_len > 0) {
        memcpy(out_pkt->payload, &src[TELEMETRY_HEADER_SIZE], out_pkt->header.payload_len);
    }

    size_t footer_idx = TELEMETRY_HEADER_SIZE + out_pkt->header.payload_len;
    uint32_t received_crc = ((uint32_t)src[footer_idx] << 24) |
                            ((uint32_t)src[footer_idx + 1] << 16) |
                            ((uint32_t)src[footer_idx + 2] << 8) |
                            ((uint32_t)src[footer_idx + 3]);

    uint32_t computed_crc = telemetry_calculate_crc32(src, TELEMETRY_HEADER_SIZE + out_pkt->header.payload_len);
    if (received_crc != computed_crc) {
        return ERR_CRC_MISMATCH;
    }

    out_pkt->crc32 = received_crc;
    return ERR_NONE;
}
