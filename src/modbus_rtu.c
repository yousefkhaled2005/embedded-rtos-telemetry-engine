/*
 * Modbus RTU Slave State Machine Implementation
 * Copyright (c) 2004-2008 Yousef Khaled.
 */

#include "telemetry/modbus_rtu.h"
#include "telemetry/packet.h"

void modbus_slave_init(modbus_rtu_slave_t *slave, uint8_t slave_id,
                       uint16_t *holding_regs, size_t num_holding,
                       uint16_t *input_regs, size_t num_input) {
    if (!slave) return;
    slave->slave_id = slave_id;
    slave->holding_registers = holding_regs;
    slave->num_holding_registers = num_holding;
    slave->input_registers = input_regs;
    slave->num_input_registers = num_input;
    slave->rx_frames_valid = 0;
    slave->rx_crc_errors = 0;
    slave->tx_frames_count = 0;
}

static int build_exception(uint8_t slave_id, uint8_t fc, uint8_t ex_code, uint8_t *resp) {
    resp[0] = slave_id;
    resp[1] = fc | 0x80;
    resp[2] = ex_code;
    uint16_t crc = telemetry_calculate_crc16(resp, 3);
    resp[3] = (uint8_t)(crc & 0xFF);
    resp[4] = (uint8_t)(crc >> 8);
    return 5;
}

int modbus_process_frame(modbus_rtu_slave_t *slave,
                          const uint8_t *req, size_t req_len,
                          uint8_t *resp, size_t resp_cap) {
    if (!slave || !req || !resp || req_len < 4) return 0;
    if (req[0] != slave->slave_id && req[0] != 0x00) return 0; // Filter non-addressed frames

    uint16_t expected_crc = ((uint16_t)req[req_len - 1] << 8) | req[req_len - 2];
    uint16_t computed_crc = telemetry_calculate_crc16(req, req_len - 2);
    if (expected_crc != computed_crc) {
        slave->rx_crc_errors++;
        return 0; // Silently drop CRC errors per Modbus RTU spec
    }

    slave->rx_frames_valid++;
    uint8_t fc = req[1];

    switch (fc) {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS: {
            uint16_t start_addr = (uint16_t)((req[2] << 8) | req[3]);
            uint16_t reg_count  = (uint16_t)((req[4] << 8) | req[5]);
            uint16_t *regs = (fc == MODBUS_FC_READ_HOLDING_REGISTERS) ?
                             slave->holding_registers : slave->input_registers;
            size_t max_regs = (fc == MODBUS_FC_READ_HOLDING_REGISTERS) ?
                             slave->num_holding_registers : slave->num_input_registers;

            if (reg_count < 1 || reg_count > 125) {
                return build_exception(slave->slave_id, fc, MODBUS_EX_ILLEGAL_DATA_VALUE, resp);
            }
            if ((start_addr + reg_count) > max_regs) {
                return build_exception(slave->slave_id, fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, resp);
            }

            resp[0] = slave->slave_id;
            resp[1] = fc;
            resp[2] = (uint8_t)(reg_count * 2);
            for (size_t i = 0; i < reg_count; ++i) {
                uint16_t val = regs[start_addr + i];
                resp[3 + i * 2]     = (uint8_t)(val >> 8);
                resp[3 + i * 2 + 1] = (uint8_t)(val & 0xFF);
            }
            size_t payload_len = 3 + reg_count * 2;
            uint16_t crc = telemetry_calculate_crc16(resp, payload_len);
            resp[payload_len]     = (uint8_t)(crc & 0xFF);
            resp[payload_len + 1] = (uint8_t)(crc >> 8);
            slave->tx_frames_count++;
            return (int)(payload_len + 2);
        }
        case MODBUS_FC_PRESET_SINGLE_REGISTER: {
            uint16_t reg_addr = (uint16_t)((req[2] << 8) | req[3]);
            uint16_t reg_val  = (uint16_t)((req[4] << 8) | req[5]);
            if (reg_addr >= slave->num_holding_registers) {
                return build_exception(slave->slave_id, fc, MODBUS_EX_ILLEGAL_DATA_ADDRESS, resp);
            }
            slave->holding_registers[reg_addr] = reg_val;
            for (int i = 0; i < 6; ++i) resp[i] = req[i];
            uint16_t crc = telemetry_calculate_crc16(resp, 6);
            resp[6] = (uint8_t)(crc & 0xFF);
            resp[7] = (uint8_t)(crc >> 8);
            slave->tx_frames_count++;
            return 8;
        }
        default:
            return build_exception(slave->slave_id, fc, MODBUS_EX_ILLEGAL_FUNCTION, resp);
    }
}
