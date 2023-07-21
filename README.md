# PIC18F-sddpi-driver
An SD SPI driver for the PIC18F46K22 microcontroller. This driver can be used to read and write to individual blocks on the card without an underlying filesystem.

## Hardware
A PIC18F microcontroller, SD card socket and and SD card are required. Pullups may also be required for the SPI data lines depending on your setup.

### Pin Assignments
The table below shows the default pin assignments.

| SD Card pin | SPI pin | PIC gpio pin |
|-------------|---------|--------------|
| D0          | MISO    | XX           |
| D3          | CS      | XX           |
| CLK         | SCK     | XX           |
| CMD         | MOSI    | XX           |

### Notes
The SSP module 1 is configured to print data over uart at 115k baud. SPI communication is handled throgh the 2nd module (SSPM2).
