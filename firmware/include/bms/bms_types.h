#ifndef BMS_TYPES_H
#define BMS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifndef BMS_CELL_COUNT
#define BMS_CELL_COUNT 9u
#endif
#define BMS_TEMPERATURE_SENSOR_COUNT 3u

typedef enum {
    BMS_STATE_INIT = 0,
    BMS_STATE_SELF_TEST,
    BMS_STATE_OFF,
    BMS_STATE_PRECHARGE,
    BMS_STATE_READY,
    BMS_STATE_CHARGE_ONLY,
    BMS_STATE_FAULT_LOCK
} bms_state_t;

typedef enum {
    BMS_FAULT_NONE = 0,
    BMS_FAULT_AFE_COMMUNICATION,
    BMS_FAULT_CELL_DATA_INVALID,
    BMS_FAULT_TEMPERATURE_DATA_INVALID,
    BMS_FAULT_CURRENT_DATA_INVALID,
    BMS_FAULT_HARDWARE_PROTECTION,
    BMS_FAULT_POWER_FEEDBACK_MISMATCH,
    BMS_FAULT_CONFIG_INVALID,
    BMS_FAULT_PRECHARGE_TIMEOUT
} bms_fault_t;

enum {
    BMS_FAULT_BIT_AFE_COMMUNICATION = 1u << 0,
    BMS_FAULT_BIT_CELL_DATA_INVALID = 1u << 1,
    BMS_FAULT_BIT_TEMPERATURE_DATA_INVALID = 1u << 2,
    BMS_FAULT_BIT_CURRENT_DATA_INVALID = 1u << 3,
    BMS_FAULT_BIT_HARDWARE_PROTECTION = 1u << 4,
    BMS_FAULT_BIT_POWER_FEEDBACK_MISMATCH = 1u << 5,
    BMS_FAULT_BIT_CELL_OVERVOLTAGE = 1u << 6,
    BMS_FAULT_BIT_CELL_UNDERVOLTAGE = 1u << 7,
    BMS_FAULT_BIT_CHARGE_OVERCURRENT = 1u << 8,
    BMS_FAULT_BIT_DISCHARGE_OVERCURRENT = 1u << 9,
    BMS_FAULT_BIT_CHARGE_TEMPERATURE = 1u << 10,
    BMS_FAULT_BIT_DISCHARGE_TEMPERATURE = 1u << 11,
    BMS_FAULT_BIT_STALE_MEASUREMENT = 1u << 12,
    BMS_FAULT_BIT_CONFIG_INVALID = 1u << 13,
    BMS_FAULT_BIT_PRECHARGE_TIMEOUT = 1u << 14
};

typedef struct {
    uint16_t cell_ov_trip_mv;
    uint16_t cell_ov_release_mv;
    uint16_t cell_uv_trip_mv;
    uint16_t cell_uv_release_mv;
    int16_t charge_temp_min_deci_c;
    int16_t charge_temp_max_deci_c;
    int16_t discharge_temp_min_deci_c;
    int16_t discharge_temp_max_deci_c;
    uint32_t charge_current_limit_ma;
    uint32_t discharge_current_limit_ma;
    uint32_t measurement_max_age_ms;
} bms_protection_config_t;

typedef struct {
    uint16_t precharge_ratio_permille;
    uint32_t precharge_timeout_ms;
} bms_power_config_t;

typedef struct {
    uint16_t cell_millivolts[BMS_CELL_COUNT];
    int32_t pack_current_milliamps;
    int16_t temperature_deci_celsius[BMS_TEMPERATURE_SENSOR_COUNT];
    uint32_t sample_sequence;
    uint32_t sample_tick;
    bool cells_valid;
    bool current_valid;
    bool temperatures_valid;
    bool afe_communication_valid;
} bms_measurement_t;

typedef struct {
    bool allow_charge;
    bool allow_discharge;
    bms_fault_t primary_fault;
    uint32_t fault_bits;
    uint32_t updated_tick;
} bms_protection_decision_t;

#endif
