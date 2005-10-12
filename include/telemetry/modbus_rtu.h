/*
 * Modbus RTU Protocol Engine for SCADA Remote Terminal Units
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#ifndef TELEMETRY_MODBUS_RTU_H
#define TELEMETRY_MODBUS_RTU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04
#define MODBUS_FC_PRESET_SINGLE_REGISTER    0x06
#define MODBUS_FC_PRESET_MULTIPLE_REGISTERS 0x10

#define MODBUS_EX_ILLEGAL_FUNCTION          0x01
#define MODBUS_EX_ILLEGAL_DATA_ADDRESS      0x02
#define MODBUS_EX_ILLEGAL_DATA_VALUE        0x03
#define MODBUS_EX_SLAVE_DEVICE_FAILURE      0x04

typedef struct {
    uint8_t   slave_id;
    uint16_t *holding_registers;
    size_t    num_holding_registers;
    uint16_t *input_registers;
    size_t    num_input_registers;
    uint32_t  rx_frames_valid;
    uint32_t  rx_crc_errors;
    uint32_t  tx_frames_count;
} modbus_rtu_slave_t;

void modbus_slave_init(modbus_rtu_slave_t *slave, uint8_t slave_id,
                       uint16_t *holding_regs, size_t num_holding,
                       uint16_t *input_regs, size_t num_input);

int  modbus_process_frame(modbus_rtu_slave_t *slave,
                          const uint8_t *request, size_t req_len,
                          uint8_t *response, size_t resp_cap);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_MODBUS_RTU_H */
