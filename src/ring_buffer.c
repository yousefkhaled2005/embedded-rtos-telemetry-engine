/*
 * Lock-Free Circular Ring Buffer Implementation
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include "telemetry/ring_buffer.h"
#include <string.h>

void ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity) {
    if (!rb || !storage || capacity == 0) return;
    rb->buffer = storage;
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->overrun_count = 0;
}

void ring_buffer_reset(ring_buffer_t *rb) {
    if (!rb) return;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

size_t ring_buffer_available(const ring_buffer_t *rb) {
    return rb ? rb->count : 0;
}

size_t ring_buffer_free_space(const ring_buffer_t *rb) {
    return rb ? (rb->capacity - rb->count) : 0;
}

bool ring_buffer_is_full(const ring_buffer_t *rb) {
    return rb ? (rb->count >= rb->capacity) : true;
}

bool ring_buffer_is_empty(const ring_buffer_t *rb) {
    return rb ? (rb->count == 0) : true;
}

bool ring_buffer_write_byte(ring_buffer_t *rb, uint8_t byte) {
    if (!rb || ring_buffer_is_full(rb)) {
        if (rb) rb->overrun_count++;
        return false;
    }
    rb->buffer[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count++;
    return true;
}

size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *src, size_t length) {
    if (!rb || !src || length == 0) return 0;
    size_t written = 0;
    while (written < length && !ring_buffer_is_full(rb)) {
        rb->buffer[rb->head] = src[written++];
        rb->head = (rb->head + 1) % rb->capacity;
        rb->count++;
    }
    if (written < length) {
        rb->overrun_count += (uint32_t)(length - written);
    }
    return written;
}

bool ring_buffer_read_byte(ring_buffer_t *rb, uint8_t *out_byte) {
    if (!rb || !out_byte || ring_buffer_is_empty(rb)) return false;
    *out_byte = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count--;
    return true;
}

size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *dest, size_t length) {
    if (!rb || !dest || length == 0) return 0;
    size_t bytes_read = 0;
    while (bytes_read < length && !ring_buffer_is_empty(rb)) {
        dest[bytes_read++] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->capacity;
        rb->count--;
    }
    return bytes_read;
}

size_t ring_buffer_peek(const ring_buffer_t *rb, uint8_t *dest, size_t length) {
    if (!rb || !dest || length == 0) return 0;
    size_t to_read = (length < rb->count) ? length : rb->count;
    size_t current_tail = rb->tail;
    for (size_t i = 0; i < to_read; ++i) {
        dest[i] = rb->buffer[current_tail];
        current_tail = (current_tail + 1) % rb->capacity;
    }
    return to_read;
}

void ring_buffer_drop(ring_buffer_t *rb, size_t length) {
    if (!rb) return;
    size_t to_drop = (length < rb->count) ? length : rb->count;
    rb->tail = (rb->tail + to_drop) % rb->capacity;
    rb->count -= to_drop;
}
