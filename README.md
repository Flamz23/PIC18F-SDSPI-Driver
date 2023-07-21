# PIC18F-sddpi-driver
An SD SPI driver for the PIC18F46K22 microcontroller. This driver can be used to read and write to individual blocks on the card without an underlying filesystem.

## Hardware
A PIC18F microcontroller, SD card socket and and SD card are required. Pullups may also be required for the SPI data lines depending on your setup.

### Pin Assignments
The table below shows the default pin assignments.

| SD Card pin | SPI pin | PIC gpio pin |
|-------------|---------|--------------|
| D0          | MISO    | Pin 20 (RD1) |
| D3          | CS      | Pin 22 (RD3) |
| CLK         | SCK     | Pin 20 (RD0) |
| CMD         | MOSI    | Pin 27 (RD4) |

### Notes
The SSP module 1 is configured to print log output over uart at 115k baud. SPI communication is handled throgh the 2nd module (SSPM2).
