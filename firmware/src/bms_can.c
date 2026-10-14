#include "bms/bms_can.h"

uint8_t bms_can_crc8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0u;
    if (data == 0) {
        return crc;
    }
    for (uint8_t i = 0u; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0u; bit < 8u; ++bit) {
            crc = (uint8_t)((crc & 0x80u) ? (uint8_t)(crc << 1) ^ 0x07u
                                          : (uint8_t)(crc << 1));
        }
    }
    return crc;
}

bool bms_can_node_id_valid(uint8_t node_id)
{
    return node_id >= BMS_CAN_NODE_ID_MIN && node_id <= BMS_CAN_NODE_ID_MAX;
}

uint16_t bms_can_tx_id(uint16_t base_id, uint8_t node_id)
{
    if (!bms_can_node_id_valid(node_id) || base_id > 0x7ffu - BMS_CAN_NODE_ID_MAX) {
        return 0u;
    }
    return (uint16_t)(base_id + node_id);
}

uint16_t bms_can_rx_command_id(uint8_t node_id)
{
    return bms_can_tx_id(0x600u, node_id);
}

bool bms_can_decode_command(uint16_t can_id,
                            uint8_t node_id,
                            const uint8_t *data,
                            uint8_t dlc,
                            bms_can_command_t *command)
{
    if (command == 0 || data == 0 || !bms_can_node_id_valid(node_id) ||
        dlc != BMS_CAN_COMMAND_DLC || can_id != bms_can_rx_command_id(node_id) ||
        data[1] != BMS_CAN_PROTOCOL_VERSION || data[3] > 3u ||
        bms_can_crc8(data, 7u) != data[7]) {
        return false;
    }

    switch (data[0]) {
    case BMS_CAN_CMD_GET_SNAPSHOT:
    case BMS_CAN_CMD_ENTER_MAINT:
    case BMS_CAN_CMD_CLEAR_FAULT:
    case BMS_CAN_CMD_DISCHARGE_REQ:
    case BMS_CAN_CMD_CHARGE_REQ:
    case BMS_CAN_CMD_ENTER_IAP:
        break;
    default:
        return false;
    }

    command->code = (bms_can_command_code_t)data[0];
    command->sequence = data[2];
    command->parameter_length = data[3];
    for (uint8_t i = 0u; i < 3u; ++i) {
        command->parameters[i] = data[4u + i];
    }
    return true;
}
