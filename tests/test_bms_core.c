#include "bms/bms_can.h"
#include "bms/bms_power_control.h"
#include "bms/bms_protection.h"

#include <assert.h>
#include <string.h>

static bms_protection_config_t valid_protection_config(void)
{
    bms_protection_config_t config = {
        3600u, 3500u, 2500u, 2800u,
        0, 450, -200, 600,
        30000u, 80000u, 200u
    };
    return config;
}

static bms_measurement_t valid_measurement(void)
{
    bms_measurement_t sample;
    memset(&sample, 0, sizeof(sample));
    for (uint32_t i = 0u; i < BMS_CELL_COUNT; ++i) {
        sample.cell_millivolts[i] = 3300u;
    }
    for (uint32_t i = 0u; i < BMS_TEMPERATURE_SENSOR_COUNT; ++i) {
        sample.temperature_deci_celsius[i] = 250;
    }
    sample.sample_tick = 100u;
    sample.cells_valid = true;
    sample.current_valid = true;
    sample.temperatures_valid = true;
    sample.afe_communication_valid = true;
    return sample;
}

static bms_protection_decision_t evaluate(bms_measurement_t *sample,
                                           bms_protection_config_t *config,
                                           uint32_t hardware_faults)
{
    bms_protection_decision_t decision;
    bms_protection_evaluate(sample, config, hardware_faults, 100u, &decision);
    return decision;
}

static void test_protection_directionality(void)
{
    bms_protection_config_t config = valid_protection_config();
    bms_measurement_t sample = valid_measurement();
    bms_protection_decision_t decision = evaluate(&sample, &config, 0u);
    assert(decision.allow_charge && decision.allow_discharge);

    sample.cell_millivolts[4] = 3600u;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && decision.allow_discharge);
    assert((decision.fault_bits & BMS_FAULT_BIT_CELL_OVERVOLTAGE) != 0u);

    sample = valid_measurement();
    sample.cell_millivolts[2] = 2500u;
    decision = evaluate(&sample, &config, 0u);
    assert(decision.allow_charge && !decision.allow_discharge);

    sample = valid_measurement();
    sample.pack_current_milliamps = -30001;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && decision.allow_discharge);

    sample = valid_measurement();
    sample.temperature_deci_celsius[1] = 500;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && decision.allow_discharge);
}

static void test_protection_fails_closed(void)
{
    bms_protection_config_t config = valid_protection_config();
    bms_measurement_t sample = valid_measurement();
    bms_protection_decision_t decision = evaluate(&sample, &config, 1u);
    assert(!decision.allow_charge && !decision.allow_discharge);
    assert((decision.fault_bits & BMS_FAULT_BIT_HARDWARE_PROTECTION) != 0u);

    config = valid_protection_config();
    config.cell_ov_release_mv = config.cell_ov_trip_mv;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && !decision.allow_discharge);
    assert((decision.fault_bits & BMS_FAULT_BIT_CONFIG_INVALID) != 0u);

    config = valid_protection_config();
    config.measurement_max_age_ms = 50u;
    sample.sample_tick = 1u;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && !decision.allow_discharge);
    assert((decision.fault_bits & BMS_FAULT_BIT_STALE_MEASUREMENT) != 0u);

    sample = valid_measurement();
    sample.afe_communication_valid = false;
    decision = evaluate(&sample, &config, 0u);
    assert(!decision.allow_charge && !decision.allow_discharge);
}

static void test_precharge_sequence(void)
{
    bms_power_control_t control;
    bms_power_config_t config = {950u, 1000u};
    bms_protection_decision_t decision = {true, true, BMS_FAULT_NONE, 0u, 0u};
    bms_power_feedback_t feedback = {false, false, false, true, true, 48000u, 10000u};
    bms_power_command_t command;
    bms_power_control_init(&control, 0u);
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 10u, &command);
    assert(control.state == BMS_STATE_PRECHARGE && !command.precharge_fet_enable);
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 20u, &command);
    assert(command.precharge_fet_enable);

    feedback.bus_millivolts = 46000u;
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 30u, &command);
    assert(control.state == BMS_STATE_READY && !command.discharge_fet_enable);

    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 40u, &command);
    assert(command.discharge_fet_enable);

    bms_power_control_init(&control, 0u);
    feedback.bus_millivolts = 10000u;
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 10u, &command);
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 20u, &command);
    bms_power_control_step(&control, BMS_POWER_REQUEST_DISCHARGE, &decision,
                           &config, &feedback, 1020u, &command);
    assert(control.state == BMS_STATE_FAULT_LOCK);
    assert(command.fault == BMS_FAULT_PRECHARGE_TIMEOUT);
    assert(!command.charge_fet_enable && !command.discharge_fet_enable);
}

static void test_can_command_validation(void)
{
    uint8_t frame[8] = {BMS_CAN_CMD_GET_SNAPSHOT, BMS_CAN_PROTOCOL_VERSION,
                        7u, 0u, 0u, 0u, 0u, 0u};
    bms_can_command_t command;
    frame[7] = bms_can_crc8(frame, 7u);
    assert(bms_can_decode_command(bms_can_rx_command_id(2u), 2u,
                                  frame, 8u, &command));
    assert(command.sequence == 7u && command.code == BMS_CAN_CMD_GET_SNAPSHOT);
    frame[2]++;
    assert(!bms_can_decode_command(bms_can_rx_command_id(2u), 2u,
                                   frame, 8u, &command));
    frame[2]--;
    assert(!bms_can_decode_command(bms_can_rx_command_id(3u), 2u,
                                   frame, 8u, &command));
    assert(!bms_can_decode_command(bms_can_rx_command_id(2u), 2u,
                                   frame, 7u, &command));
    assert(!bms_can_node_id_valid(0u));
}

int main(void)
{
    test_protection_directionality();
    test_protection_fails_closed();
    test_precharge_sequence();
    test_can_command_validation();
    return 0;
}
