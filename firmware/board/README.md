# BQ76940 Development Board

This port is based on `BQ76940/2.原理图/BQ76940_CAN_20210906.pdf` and the accompanying BMS specification. The schematic labels the MCU as STM32F103C8T6. The user confirmed the photographed board is connected to a 9-cell pack. Vendor source is a reference only; its Keil project selects STM32F103C6 and its latest sample firmware is configured for 15 cells. The product requirements document separately specifies a 13S LiFePO4 target.

## Confirmed connections

| Signal | MCU / component | Evidence |
|---|---|---|
| BQ76940 SCL | PB8 | Schematic net `MCU_IIC1_SCL`; vendor I2C header |
| BQ76940 SDA | PB9 | Schematic net `MCU_IIC1_SDA`; vendor I2C header |
| CAN1 RX | PA11 | Schematic net `CAN_RX`, routed through TJA1050 |
| CAN1 TX | PA12 | Schematic net `CAN_TX`, routed through TJA1050 |
| CAN transceiver | TJA1050, 5 V side | Schematic U5 |
| Battery monitor | BQ76940 / BQ7694003 | Schematic U6; I2C address documented by vendor as 0x08 (7-bit) |
| Temperature inputs | TS1/TS2/TS3 with external NTC support | BQ76940 pins on schematic; vendor guide describes 10 kOhm, B=3950 NTC |
| Cell taps | Labeled BQ76940 battery connector into VC0..VC15 filter network | User confirms a 9S pack is connected; exact connector pin-to-VC mapping and unused-channel treatment still need electrical verification |

The vendor sample uses software-driven open-drain I2C on PB8/PB9. Keep this pin mapping, but verify the selected I2C peripheral mode and bus timing against the board pull-ups before enabling it.

## Hardware capability boundary

- The schematic has CHG/DSG driven from the BQ76940 pins through the board power stage. It does not show a BQ76200 driver, BQ34Z100-G1 gauge, or a separate precharge branch.
- The MCU schematic does not expose independent CHG/DSG control GPIOs. Firmware must not claim the demand-document PowerControlTask can command MOSFETs until the actual board circuitry and control path are proven.
- The vendor specification calls this a development board and recommends keeping current at or below 10 A. It is not evidence of support for the demand-document's 35 A charging reference or an AMR traction current.
- The connected test pack is 9S, while the product requirements target is 13S and the supplied source also has a 15-cell variant. These are separate configurations; verify the physical harness and per-channel mapping before using either profile.

No production thresholds, current calibration, cell-channel mapping, or FET command polarity is frozen here. Confirm these with the physical board, cell datasheet, schematic/netlist, and current-limited tests.
