# Hardware Bring-Up Checklist

- [ ] Confirm the board revision and read its schematic/BOM.
- [ ] Confirm the exact MCU part number and available Flash/RAM.
- [ ] Verify the BQ76940 supply, I2C address, ALERT polarity, and pull-ups.
- [ ] Map the 9 cell taps to VC pins; verify monotonic voltages and unused-channel handling with a current-limited source.
- [ ] Before product-target testing, validate the separate 13S profile and its VC mapping.
- [ ] Confirm shunt resistance, Kelvin routing, current polarity, and calibration.
- [ ] Confirm NTC type, divider circuit, and open/short detection range.
- [ ] Trace CHG/DSG/precharge control through the gate driver and power FETs.
- [ ] Verify the hardware-safe state with the MCU held in reset.
- [ ] Record validated protection thresholds and recovery hysteresis from cell/pack specifications.
- [ ] Confirm CAN physical-layer transceiver, termination, and bit timing.
