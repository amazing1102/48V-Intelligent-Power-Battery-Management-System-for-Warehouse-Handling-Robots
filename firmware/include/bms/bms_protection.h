#ifndef BMS_PROTECTION_H
#define BMS_PROTECTION_H

#include "bms_types.h"

void bms_protection_init(void);
bool bms_protection_config_valid(const bms_protection_config_t *config);
void bms_protection_evaluate(const bms_measurement_t *measurement,
                             const bms_protection_config_t *config,
                             uint32_t hardware_faults,
                             uint32_t now_tick,
                             bms_protection_decision_t *decision);

#endif
