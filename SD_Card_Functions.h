// December 2017
//
// USART setup and LCD functions
// For NewHaven 4x20 LCD Display
//


/*
// FUNCTION PROTOTYPES
void spi_setup();
void spi_transfer(char dat);
void SD_powerup(int i);
void SD_command(short cmd, int arg, short crc);
char SD_readRes1();
void SD_readRes3_7(char *res);
char SD_enterIdleState();
void SD_sendIfCond(char *res);
void SD_readOCR(char *res);
void SD_sendApp();
char SD_sendOpCond();
void SD_Initialize();
char SD_readSingleBlock(int addr, char *buf, char *token);
char SD_writeSingleBlock(int addr, char *buf, char *token);
void testRead(char addr);
void testWrite(char addr, char *buf);
// LOGGING
void send_data2(char c);
void RS232_setup2();
void signed_write(int k); // write 4 digit signed number to serial
void print_UART(char *text);
void print_UART_hex(char val);
void SD_parseR1(char res);
void SD_parseR3(char *res);
void SD_parseR7(char *res);
*/

// V.20
// FUNCTION PROTOTYPES
void spi_setup();
char spi_transfer(char d);
void RS232_setup2();
void send_data2(char c);
void print_UART_hex(char val);
void print_UART(char *text);
void SD_command(short cmd, int arg, short crc);
char SD_readR1();
void SD_readR3_7(char *r);
void SD_Initialize();
void SD_readBlock(char *r1, int addr, char *buf, char *token);
void SD_writeBlock(char *r1, int addr, char *buf, char *token);
void signed_write(int p, int q);