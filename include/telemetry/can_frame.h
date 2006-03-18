/*
 * Controller Area Network (CAN 2.0B) Telemetry Framing and Filter
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#ifndef TELEMETRY_CAN_FRAME_H
#define TELEMETRY_CAN_FRAME_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_MAX_DLC 8

typedef struct {
    uint32_t id;         // Standard (11-bit) or Extended (29-bit) ID
    bool     is_extended;
    bool     is_rtr;
    uint8_t  dlc;        // Data length code (0 - 8)
    uint8_t  data[CAN_MAX_DLC];
    uint32_t timestamp_us;
} can_frame_t;

typedef struct {
    uint32_t mask;
    uint32_t filter_id;
    bool     is_extended;
} can_filter_bank_t;

bool can_frame_matches_filter(const can_frame_t *frame, const can_filter_bank_t *filter);
void can_unpack_sensor_data(const can_frame_t *frame, uint16_t *channel, int16_t *raw_val, uint16_t *flags);
void can_pack_sensor_data(can_frame_t *frame, uint32_t id, uint16_t channel, int16_t raw_val, uint16_t flags);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_CAN_FRAME_H */
