/*
 * CAN 2.0B Hardware Filter Acceptance Simulation
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include "telemetry/can_frame.h"

bool can_frame_matches_filter(const can_frame_t *frame, const can_filter_bank_t *filter) {
    if (!frame || !filter) return false;
    if (frame->is_extended != filter->is_extended) return false;
    return (frame->id & filter->mask) == (filter->filter_id & filter->mask);
}

void can_unpack_sensor_data(const can_frame_t *frame, uint16_t *channel, int16_t *raw_val, uint16_t *flags) {
    if (!frame || frame->dlc < 6) return;
    if (channel) *channel = (uint16_t)((frame->data[0] << 8) | frame->data[1]);
    if (raw_val) *raw_val = (int16_t)((frame->data[2] << 8) | frame->data[3]);
    if (flags)   *flags   = (uint16_t)((frame->data[4] << 8) | frame->data[5]);
}

void can_pack_sensor_data(can_frame_t *frame, uint32_t id, uint16_t channel, int16_t raw_val, uint16_t flags) {
    if (!frame) return;
    frame->id = id;
    frame->is_extended = (id > 0x7FF);
    frame->is_rtr = false;
    frame->dlc = 6;
    frame->data[0] = (uint8_t)(channel >> 8);
    frame->data[1] = (uint8_t)(channel & 0xFF);
    frame->data[2] = (uint8_t)((uint16_t)raw_val >> 8);
    frame->data[3] = (uint8_t)((uint16_t)raw_val & 0xFF);
    frame->data[4] = (uint8_t)(flags >> 8);
    frame->data[5] = (uint8_t)(flags & 0xFF);
}
