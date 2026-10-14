#include "bms/bms_power_control.h"

bool bms_power_config_valid(const bms_power_config_t *config)
{
    return config != 0 && config->precharge_ratio_permille > 0u &&
           config->precharge_ratio_permille <= 1000u &&
           config->precharge_timeout_ms > 0u;
}

void bms_power_control_init(bms_power_control_t *control, uint32_t now_tick)
{
    if (control != 0) {
        control->state = BMS_STATE_OFF;
        control->state_entered_tick = now_tick;
    }
}

static void enter_state(bms_power_control_t *control, bms_state_t state, uint32_t now_tick)
{
    if (control->state != state) {
        control->state = state;
        control->state_entered_tick = now_tick;
    }
}

static bool decision_requires_lock(const bms_protection_decision_t *decision)
{
    const uint32_t lock_faults = BMS_FAULT_BIT_AFE_COMMUNICATION |
                                 BMS_FAULT_BIT_CELL_DATA_INVALID |
                                 BMS_FAULT_BIT_TEMPERATURE_DATA_INVALID |
                                 BMS_FAULT_BIT_CURRENT_DATA_INVALID |
                                 BMS_FAULT_BIT_HARDWARE_PROTECTION |
                                 BMS_FAULT_BIT_STALE_MEASUREMENT |
                                 BMS_FAULT_BIT_CONFIG_INVALID |
                                 BMS_FAULT_BIT_POWER_FEEDBACK_MISMATCH |
                                 BMS_FAULT_BIT_PRECHARGE_TIMEOUT;
    return decision == 0 || (decision->fault_bits & lock_faults) != 0u ||
           decision->primary_fault != BMS_FAULT_NONE;
}

void bms_power_control_step(bms_power_control_t *control,
                            bms_power_request_t request,
                            const bms_protection_decision_t *decision,
                            const bms_power_config_t *config,
                            const bms_power_feedback_t *feedback,
                            uint32_t now_tick,
                            bms_power_command_t *command)
{
    if (command == 0) {
        return;
    }
    command->charge_fet_enable = false;
    command->discharge_fet_enable = false;
    command->precharge_fet_enable = false;
    command->fault = BMS_FAULT_NONE;

    if (control == 0 || !bms_power_config_valid(config)) {
        command->fault = BMS_FAULT_CONFIG_INVALID;
        return;
    }
    if (feedback != 0 && feedback->feedback_valid && !feedback->feedback_matches_command) {
        enter_state(control, BMS_STATE_FAULT_LOCK, now_tick);
        command->fault = BMS_FAULT_POWER_FEEDBACK_MISMATCH;
    }
    if (decision_requires_lock(decision)) {
        enter_state(control, BMS_STATE_FAULT_LOCK, now_tick);
    }

    switch (control->state) {
    case BMS_STATE_OFF:
        if (decision_requires_lock(decision)) {
            enter_state(control, BMS_STATE_FAULT_LOCK, now_tick);
        } else if (request == BMS_POWER_REQUEST_CHARGE && decision->allow_charge) {
            enter_state(control, BMS_STATE_CHARGE_ONLY, now_tick);
        } else if (request == BMS_POWER_REQUEST_DISCHARGE && decision->allow_discharge) {
            enter_state(control, BMS_STATE_PRECHARGE, now_tick);
        }
        break;
    case BMS_STATE_PRECHARGE:
        if (request != BMS_POWER_REQUEST_DISCHARGE || !decision->allow_discharge) {
            enter_state(control, BMS_STATE_OFF, now_tick);
        } else if (feedback == 0 || !feedback->feedback_valid ||
                   feedback->pack_millivolts == 0u) {
            enter_state(control, BMS_STATE_FAULT_LOCK, now_tick);
            command->fault = BMS_FAULT_POWER_FEEDBACK_MISMATCH;
        } else if ((uint32_t)feedback->bus_millivolts * 1000u >=
                   (uint32_t)feedback->pack_millivolts * config->precharge_ratio_permille) {
            enter_state(control, BMS_STATE_READY, now_tick);
        } else if ((uint32_t)(now_tick - control->state_entered_tick) >=
                   config->precharge_timeout_ms) {
            enter_state(control, BMS_STATE_FAULT_LOCK, now_tick);
            command->fault = BMS_FAULT_PRECHARGE_TIMEOUT;
        } else {
            command->precharge_fet_enable = true;
        }
        break;
    case BMS_STATE_READY:
        if (request != BMS_POWER_REQUEST_DISCHARGE || !decision->allow_discharge) {
            enter_state(control, BMS_STATE_OFF, now_tick);
        } else {
            command->discharge_fet_enable = true;
        }
        break;
    case BMS_STATE_CHARGE_ONLY:
        if (request != BMS_POWER_REQUEST_CHARGE || !decision->allow_charge) {
            enter_state(control, BMS_STATE_OFF, now_tick);
        } else {
            command->charge_fet_enable = true;
        }
        break;
    case BMS_STATE_FAULT_LOCK:
    case BMS_STATE_INIT:
    case BMS_STATE_SELF_TEST:
    default:
        break;
    }
}
