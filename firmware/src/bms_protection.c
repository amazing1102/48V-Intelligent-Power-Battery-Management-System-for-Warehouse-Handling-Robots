#include "bms/bms_protection.h"

void bms_protection_init(void)
{
}

bool bms_protection_config_valid(const bms_protection_config_t *config)
{
    return config != 0 &&
           config->cell_ov_trip_mv > config->cell_ov_release_mv &&
           config->cell_uv_trip_mv < config->cell_uv_release_mv &&
           config->cell_uv_trip_mv < config->cell_ov_trip_mv &&
           config->charge_temp_min_deci_c < config->charge_temp_max_deci_c &&
           config->discharge_temp_min_deci_c < config->discharge_temp_max_deci_c &&
           config->charge_current_limit_ma > 0u &&
           config->discharge_current_limit_ma > 0u &&
           config->measurement_max_age_ms > 0u;
}

static void set_primary_fault(bms_protection_decision_t *decision)
{
    if (decision->primary_fault != BMS_FAULT_NONE) {
        return;
    }
    if ((decision->fault_bits & BMS_FAULT_BIT_AFE_COMMUNICATION) != 0u) {
        decision->primary_fault = BMS_FAULT_AFE_COMMUNICATION;
    } else if ((decision->fault_bits & BMS_FAULT_BIT_HARDWARE_PROTECTION) != 0u) {
        decision->primary_fault = BMS_FAULT_HARDWARE_PROTECTION;
    } else if ((decision->fault_bits & BMS_FAULT_BIT_CELL_DATA_INVALID) != 0u) {
        decision->primary_fault = BMS_FAULT_CELL_DATA_INVALID;
    } else if ((decision->fault_bits & BMS_FAULT_BIT_CURRENT_DATA_INVALID) != 0u) {
        decision->primary_fault = BMS_FAULT_CURRENT_DATA_INVALID;
    } else if ((decision->fault_bits & BMS_FAULT_BIT_TEMPERATURE_DATA_INVALID) != 0u) {
        decision->primary_fault = BMS_FAULT_TEMPERATURE_DATA_INVALID;
    }
}

void bms_protection_evaluate(const bms_measurement_t *measurement,
                             const bms_protection_config_t *config,
                             uint32_t hardware_faults,
                             uint32_t now_tick,
                             bms_protection_decision_t *decision)
{
    if (decision == 0) {
        return;
    }

    decision->allow_charge = false;
    decision->allow_discharge = false;
    decision->primary_fault = BMS_FAULT_NONE;
    decision->fault_bits = 0u;
    decision->updated_tick = now_tick;

    if (!bms_protection_config_valid(config)) {
        decision->fault_bits |= BMS_FAULT_BIT_CONFIG_INVALID;
        return;
    }
    if (measurement == 0) {
        decision->fault_bits = BMS_FAULT_BIT_CELL_DATA_INVALID |
                               BMS_FAULT_BIT_CURRENT_DATA_INVALID |
                               BMS_FAULT_BIT_TEMPERATURE_DATA_INVALID;
        decision->primary_fault = BMS_FAULT_CELL_DATA_INVALID;
        return;
    }
    if (!measurement->afe_communication_valid) {
        decision->fault_bits |= BMS_FAULT_BIT_AFE_COMMUNICATION;
    }
    if ((uint32_t)(now_tick - measurement->sample_tick) > config->measurement_max_age_ms) {
        decision->fault_bits |= BMS_FAULT_BIT_STALE_MEASUREMENT;
    }
    if (hardware_faults != 0u) {
        decision->fault_bits |= BMS_FAULT_BIT_HARDWARE_PROTECTION;
    }
    if (!measurement->cells_valid) {
        decision->fault_bits |= BMS_FAULT_BIT_CELL_DATA_INVALID;
    }
    if (!measurement->current_valid) {
        decision->fault_bits |= BMS_FAULT_BIT_CURRENT_DATA_INVALID;
    }
    if (!measurement->temperatures_valid) {
        decision->fault_bits |= BMS_FAULT_BIT_TEMPERATURE_DATA_INVALID;
    }

    set_primary_fault(decision);
    if (decision->fault_bits != 0u) {
        return;
    }

    decision->allow_charge = true;
    decision->allow_discharge = true;
    for (uint32_t i = 0u; i < BMS_CELL_COUNT; ++i) {
        if (measurement->cell_millivolts[i] >= config->cell_ov_trip_mv) {
            decision->fault_bits |= BMS_FAULT_BIT_CELL_OVERVOLTAGE;
            decision->allow_charge = false;
        }
        if (measurement->cell_millivolts[i] <= config->cell_uv_trip_mv) {
            decision->fault_bits |= BMS_FAULT_BIT_CELL_UNDERVOLTAGE;
            decision->allow_discharge = false;
        }
    }

    if (measurement->pack_current_milliamps < 0 &&
        (uint32_t)(-(int64_t)measurement->pack_current_milliamps) > config->charge_current_limit_ma) {
        decision->fault_bits |= BMS_FAULT_BIT_CHARGE_OVERCURRENT;
        decision->allow_charge = false;
    }
    if (measurement->pack_current_milliamps > 0 &&
        (uint32_t)measurement->pack_current_milliamps > config->discharge_current_limit_ma) {
        decision->fault_bits |= BMS_FAULT_BIT_DISCHARGE_OVERCURRENT;
        decision->allow_discharge = false;
    }

    for (uint32_t i = 0u; i < BMS_TEMPERATURE_SENSOR_COUNT; ++i) {
        int16_t temperature = measurement->temperature_deci_celsius[i];
        if (temperature < config->charge_temp_min_deci_c ||
            temperature > config->charge_temp_max_deci_c) {
            decision->fault_bits |= BMS_FAULT_BIT_CHARGE_TEMPERATURE;
            decision->allow_charge = false;
        }
        if (temperature < config->discharge_temp_min_deci_c ||
            temperature > config->discharge_temp_max_deci_c) {
            decision->fault_bits |= BMS_FAULT_BIT_DISCHARGE_TEMPERATURE;
            decision->allow_discharge = false;
        }
    }
}
