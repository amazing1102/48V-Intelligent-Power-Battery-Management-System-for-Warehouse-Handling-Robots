#ifndef BMS_CAN_H
#define BMS_CAN_H

#include <stdbool.h>
#include <stdint.h>

#define BMS_CAN_NODE_ID_MIN 1u
#define BMS_CAN_NODE_ID_MAX 15u
#define BMS_CAN_COMMAND_DLC 8u
#define BMS_CAN_PROTOCOL_VERSION 1u

typedef enum {
    BMS_CAN_CMD_GET_SNAPSHOT = 0x01,
    BMS_CAN_CMD_ENTER_MAINT = 0x02,
    BMS_CAN_CMD_CLEAR_FAULT = 0x03,
    BMS_CAN_CMD_DISCHARGE_REQ = 0x10,
    BMS_CAN_CMD_CHARGE_REQ = 0x11,
    BMS_CAN_CMD_ENTER_IAP = 0x20
} bms_can_command_code_t;

typedef struct {
    bms_can_command_code_t code;
    uint8_t sequence;
    uint8_t parameter_length;
    uint8_t parameters[3];
} bms_can_command_t;

uint8_t bms_can_crc8(const uint8_t *data, uint8_t length);
bool bms_can_node_id_valid(uint8_t node_id);
uint16_t bms_can_tx_id(uint16_t base_id, uint8_t node_id);
uint16_t bms_can_rx_command_id(uint8_t node_id);
bool bms_can_decode_command(uint16_t can_id,
                            uint8_t node_id,
                            const uint8_t *data,
                            uint8_t dlc,
                            bms_can_command_t *command);

#endif
