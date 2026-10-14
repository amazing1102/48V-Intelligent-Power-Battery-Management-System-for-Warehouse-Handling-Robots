#ifndef BMS_POWER_CONTROL_H
#define BMS_POWER_CONTROL_H

#include "bms_types.h"

typedef enum {
    BMS_POWER_REQUEST_NONE = 0,
    BMS_POWER_REQUEST_CHARGE,
    BMS_POWER_REQUEST_DISCHARGE,
    BMS_POWER_REQUEST_SHUTDOWN
} bms_power_request_t;

typedef struct {
    bool charge_fet_on;
    bool discharge_fet_on;
    bool precharge_fet_on;
    bool feedback_valid;
    bool feedback_matches_command;
    uint16_t pack_millivolts;
    uint16_t bus_millivolts;
} bms_power_feedback_t;

typedef struct {
    uint32_t state_entered_tick;
    bms_state_t state;
} bms_power_control_t;

typedef struct {
    bool charge_fet_enable;
    bool discharge_fet_enable;
    bool precharge_fet_enable;
    bms_fault_t fault;
} bms_power_command_t;

bool bms_power_config_valid(const bms_power_config_t *config);
void bms_power_control_init(bms_power_control_t *control, uint32_t now_tick);
void bms_power_control_step(bms_power_control_t *control,
                            bms_power_request_t request,
                            const bms_protection_decision_t *decision,
                            const bms_power_config_t *config,
                            const bms_power_feedback_t *feedback,
                            uint32_t now_tick,
                            bms_power_command_t *command);

#endif
