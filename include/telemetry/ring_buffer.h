/*
 * Lock-Free Circular Ring Buffer for Embedded Systems
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#ifndef TELEMETRY_RING_BUFFER_H
#define TELEMETRY_RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buffer;
    size_t   capacity;
    size_t   head;
    size_t   tail;
    size_t   count;
    uint32_t overrun_count;
} ring_buffer_t;

void   ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity);
void   ring_buffer_reset(ring_buffer_t *rb);
size_t ring_buffer_available(const ring_buffer_t *rb);
size_t ring_buffer_free_space(const ring_buffer_t *rb);
bool   ring_buffer_is_full(const ring_buffer_t *rb);
bool   ring_buffer_is_empty(const ring_buffer_t *rb);

bool   ring_buffer_write_byte(ring_buffer_t *rb, uint8_t byte);
size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *src, size_t length);
bool   ring_buffer_read_byte(ring_buffer_t *rb, uint8_t *out_byte);
size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *dest, size_t length);
size_t ring_buffer_peek(const ring_buffer_t *rb, uint8_t *dest, size_t length);
void   ring_buffer_drop(ring_buffer_t *rb, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_RING_BUFFER_H */
