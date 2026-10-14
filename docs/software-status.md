# Software Status

## Implemented and host-tested

- Protection configuration validation; no operational thresholds are supplied by default.
- Fail-closed handling for invalid configuration, AFE communication loss, invalid/old samples, hardware protection, and invalid current or temperature data.
- Direction-specific cell voltage, current, and temperature decisions.
- Single-owner power state machine with charge-only, precharge, ready, and fault-lock behavior. Board drivers are intentionally not connected.
- Standard 11-bit CAN command ID selection and 8-byte command validation (node, protocol version, command, parameter length, CRC-8).
- Host unit tests for valid decisions, direction-specific blocks, hard faults, stale data, precharge sequence, and command-frame rejection.

The current CAN CRC is CRC-8/ATM: polynomial `0x07`, initial value `0x00`, no reflection, no final XOR, over bytes 0 through 6. Confirm this against the vehicle network definition before integration.

## Confirmed board facts

- The supplied CAN schematic labels the MCU STM32F103C8T6. The latest supplied Keil source project instead selects STM32F103C6; use C8T6 for the new project.
- I2C is routed to PB8/PB9 and CAN1 to PA11/PA12 through a TJA1050.
- The vendor calls the board a development board and recommends current at or below 10 A.
- The user confirmed the photographed board is connected to a 9S pack. Core logic defaults to 9 cells; the product requirements define a separate 13S LiFePO4 target.
- The schematic does not include the demand-document BQ34Z100-G1, BQ76200, or precharge circuit. Its CHG/DSG pins appear to drive the board FET stage directly, without MCU-controlled FET GPIOs.

## Pending hardware-dependent work

- Confirm the physical board matches the supplied schematic and identify its exact revision.
- Confirm the 9S tap population and cell-channel mapping on the physical connector; separately validate unused channels and the 13S product profile before production-target testing.
- Trace the exact CHG/DSG FET path and feedback signals; decide whether this board can support the requested power-control workflow.
- Import/configure the CubeMX project and Keil target for C8T6 using confirmed board pins.
- Implement BQ76940 register transport, the actual 9S VC mapping, calibration, ALERT handling, and bus recovery; validate the 13S product profile separately.
- Confirm whether BQ34Z100-G1 and BQ76200 are present; implement gauge and gate-driver transports as applicable.
- Set cell, current, and thermal limits from the selected cell datasheet, shunt, MOSFETs, and measured AMR load.
- Add FreeRTOS tasks, queues/mutexes, watchdog supervision, balancing, persistence, fault log, IAP, and full CAN telemetry/ACK behavior.
- Verify each item on a current-limited simulator before connecting a real battery or load.
