#include "SD_Card_Functions.h"

// SD Commands
#define CMD0                0
#define CMD0_ARG            0x00000000
#define CMD0_CRC            0x94
#define CMD8                8
#define CMD8_ARG            0x0000001AA
#define CMD8_CRC            0x86
#define CMD58               58
#define CMD58_ARG           0x00000000
#define CMD58_CRC           0x00
#define CMD55               55
#define CMD55_ARG           0x00000000
#define CMD55_CRC           0x00
#define ACMD41              41
#define ACMD41_ARG          0x40FF8000
#define ACMD41_CRC          0x00
#define CMD17               17
#define CMD17_CRC           0x00
#define SD_MAX_READ_ATTEMPTS 12500
#define CMD24               24
#define CMD24_CRC           0x00
#define SD_MAX_WRITE_ATTEMPTS 31250

#define SD_MAX_INIT_ATTEMPTS 100
#define SD_START_TOKEN      0xfe
#define SD_BLOCK_LEN        512


//------------------------------------------------------------------------------
void CS_ENABLE()
{
     latd.f3=0;
}
//------------------------------------------------------------------------------
void CS_DISABLE()
{
     latd.f3=1;
}
//------------------------------------------------------------------------------
void spi_setup()            // Set up SSP1 module
{
     trisd.f0=0;            // make SCK2 a digital output (pin #19)
     anseld.f0=0;
     trisd.f1=1;            // make SDI1 a digital input (pin #20)
     anseld.f1=0;
     trisd.f3=0;            // make SS2 a digital output (pin #22)
     anseld.f3=0;
     trisd.f4=0;            // make SD02 a digital output (pin #27)
     anseld.f4=0;

     ssp2stat.smp=0;        // Input data sampled at middle of data output time
     ssp2stat.cke=1;        // transmit on clk idle->active
     ssp2con1=0x02;         // SPI Master mode(bits 3-0)
     ssp2con1.sspen=1;      // SSP enable (bit 5)
     ssp2con1.ckp=0;        // clk active high (bit 4)
     ssp2add=159;           // set up spi frequency of 100,000 Hz

     pir3.ssp2if=0;         // reset flag
     latd.f3=1;             // ss2 active low
}
//------------------------------------------------------------------------------
char spi_transfer(char d) // write to SPI buffer
{
     ssp2buf=d;           // load register address
     while(pir3.ssp2if==0){} // wait for interrupt flag to be set
     pir3.ssp2if=0;          // clear interrupt flag
     return ssp2buf;
}
//------------------------------------------------------------------------------
void RS232_setup2()         // Setup for USART2 (Tx pin #29)
{
// Set up for asynchronous transmission at 9600 Baud
     trisd.f7=1;
     anseld.f7=0;
     trisd.f6=1;
     anseld.f6=0;           // set up RD6 & RD7 as digital inputs
     txsta2.txen=1;         // tx enable
     rcsta2.spen=1;         // ser port enable
     spbrgh2=0;
     spbrg2=34;             // 115.2k baud @64MHz
     txsta2.sync=0;         // asynchronous mode
     txsta2.brgh=1;         // high speed baud rate
     baudcon2.brg16=0;      // 8-bit Baud Rate Generator
     delay_ms(1000);        // allow time for Baud rate generator to come on
}
//------------------------------------------------------------------------------
void send_data2(char c)     // Send to USART2 (pin #29)
{
     txreg2=c;              // send this byte to the USB
     while(~txsta2.trmt){}  // check trmt bit. Wait for transmission to be finished
}
//------------------------------------------------------------------------------
void print_UART_hex(char val) // print hex byte to uart
{
    char u_nibble=(val&0xf0)>>4;
    char l_nibble=val&0x0f; // extract upper and lower nibbles from input value

    u_nibble += (u_nibble > 9) ? 55 : 48;
    l_nibble += (l_nibble > 9) ? 55 : 48;   // convert to ascii
    send_data2(u_nibble);
    send_data2(l_nibble);                   // send result to UART

    // char buffer[8];
    // sprintf(buffer, "%02C ", val);
    // send_data2(buffer);
}
//------------------------------------------------------------------------------
void print_UART(char *text)
{
     char len,n;
     len=strlen(text);                      // find length of string
     for(n=0;n<len;n++) send_data2(text[n]); //send string characters one by one
}
//------------------------------------------------------------------------------
void SD_command(short cmd, int arg, short crc)
{
    spi_transfer(cmd|0x40);                 // write command to sd card (bit 6 = 1)
    spi_transfer((char)(arg>>24));
    spi_transfer((char)(arg>>16));
    spi_transfer((char)(arg>>8));
    spi_transfer((char)(arg));              // write argument in four 8 bit segments
    spi_transfer(crc|0x01);                 // write crc  (bit 0 = 1)
}
//------------------------------------------------------------------------------
char SD_readR1()
{
    char i=0, r1=0xff;
    while(r1==0xff) // poll until data received
    {
        r1=spi_transfer(0xff);              // write dummy data
        i++;
        if(i > 8) break;                    // exit on no response   
    }
    return r1;
}
//------------------------------------------------------------------------------
void SD_readR3_7(char *r)
{
    char i;
    r[0]=SD_readR1();
    for(i=1;i<5;i++) r[i]=spi_transfer(0xff); // load response in buffer pointer 
}
//------------------------------------------------------------------------------
void SD_Initialize()
{
    char i, r1, r3_7[5];
    CS_DISABLE();
    print_UART("SanDisk 8GB SDHC\r\n");
    delay_ms(1);                            // give SD card time to power up


    /*************************************************************** 
     * Power up sequence 
     * *************************************************************/
    spi_transfer(0xff);
    CS_ENABLE();                            // select SD card

    print_UART("Power on Sequence\r\n");
    for(i=0;i<10;i++) spi_transfer(0xff);   // send >74 clock cycles to synchronize

    CS_DISABLE();
    spi_transfer(0xff);


    /*************************************************************** 
     * Command to Idle state 
     * *************************************************************/
    spi_transfer(0xff);
    CS_ENABLE();                            // assert chip select
    spi_transfer(0xff);

    print_UART("Sending CMD0...\r\n");
    SD_command(CMD0, CMD0_ARG, CMD0_CRC);   // send CMD 0

    r1=SD_readR1();                         // read R1 response
    print_UART("\tResponse: ");             // write R1 response to UART
    print_UART_hex(r1);
    print_UART("\r\n");

    spi_transfer(0xff);
    CS_DISABLE();                           // deassert chip select
    spi_transfer(0xff);

    /*************************************************************** 
     * Send Interface condition (voltage check) 
     * *************************************************************/
    spi_transfer(0xff);
    CS_ENABLE();                            // assert chip select
    spi_transfer(0xff);

    print_UART("Sending CMD8...\r\n");
    SD_command(CMD8, CMD8_ARG, CMD8_CRC);   // send CMD 8

    SD_readR3_7(r3_7);                      // read R7 response
    print_UART("\tResponse: ");             // write R7 response to UART
    print_UART_hex(r3_7[0]);
    print_UART(" | ");
    print_UART_hex(r3_7[1]);
    print_UART_hex(r3_7[2]);
    print_UART_hex(r3_7[3]);
    print_UART_hex(r3_7[4]);
    print_UART("\r\n");

    spi_transfer(0xff);
    CS_DISABLE();                           // deassert chip select
    spi_transfer(0xff);


    /***************************************************************** 
     * Send READ_OCR (Request acceptable VDD, Capacity & Status) 
     * ***************************************************************/
    spi_transfer(0xff);
    CS_ENABLE();                            // assert chip select
    spi_transfer(0xff);

    print_UART("Sending CMD58...\r\n");
    SD_command(CMD58, CMD58_ARG, CMD58_CRC); // send CMD 58

    SD_readR3_7(r3_7);                      // read R3 response
    print_UART("\tResponse: ");             // write R7 response to UART
    print_UART_hex(r3_7[0]);
    print_UART(" | ");
    print_UART_hex(r3_7[1]);
    print_UART_hex(r3_7[2]);
    print_UART_hex(r3_7[3]);
    print_UART_hex(r3_7[4]);
    print_UART("\r\n");

    spi_transfer(0xff);
    CS_DISABLE();                           // deassert chip select
    spi_transfer(0xff);


    /***************************************************************** 
     * Initialize Card [Send CMD55 (APP_CMD) and ACMD41 (SEND_OP_COND) 
     * continuously until card is ready r1=0x00)
     * ***************************************************************/
    i=0;
    while(i<SD_MAX_INIT_ATTEMPTS)
    {
        spi_transfer(0xff);
        CS_ENABLE();                            // assert chip select
        spi_transfer(0xff);

        print_UART("Sending CMD55...\r\n");
        SD_command(CMD55, CMD55_ARG, CMD55_CRC); // send CMD 55

        r1=SD_readR1();                         // read R1 response
        print_UART("\tResponse: ");             // write R1 response to UART
        print_UART_hex(r1);
        print_UART("\r\n");

        spi_transfer(0xff);
        CS_DISABLE();                           // deassert chip select
        spi_transfer(0xff);

        if(r1<2)                                // Send AMCD41 if no error on CMD55
        {
            spi_transfer(0xff);
            CS_ENABLE();                        // assert chip select
            spi_transfer(0xff);

            print_UART("Sending ACMD41...\r\n");
            SD_command(ACMD41, ACMD41_ARG, ACMD41_CRC); // send ACMD 41

            r1=SD_readR1();                     // read R1 response
            print_UART("\tResponse: ");         // write R1 response to UART
            print_UART_hex(r1);
            print_UART("\r\n");

            spi_transfer(0xff);
            CS_DISABLE();                       // deassert chip select
            spi_transfer(0xff);

            if (r1==0x00) {print_UART("Card Initialized... \r\n"); return;}    
        }
        else 
        {
            print_UART("Initialization Error...\r\n");
            return;    
        }

        i++;
        delay_ms(10);
    }
}
//------------------------------------------------------------------------------
//                              Read Block
//------------------------------------------------------------------------------
/*******************************************************************************
 Read single 512 byte block
 token = 0xfe - Successful read
 token = 0x0x - Data error
 token = 0xff - Timeout
*******************************************************************************/
void SD_readBlock(char *r1, int addr, char *buf, char *token)
{
    int i=0;
    char read, val;
    *token=0xff;                            // Clear token
    spi_transfer(0xff);
    CS_ENABLE();                            // assert chip select
    spi_transfer(0xff);

    SD_command(CMD17, addr, CMD17_CRC);     // send CMD 17 (READ_BLOCCK) with starting address
    *r1=SD_readR1();                        // read R1 response

    if(*r1!=0xff)                           // continue on valid response
    {
        while(i<SD_MAX_READ_ATTEMPTS)       // poll for R1 response (timeout = 100ms)
        {
            read=spi_transfer(0xff);
            if (read!=0xff) break;          // exit on valid response                 
            i++;
        }
        
        if(read==SD_START_TOKEN)            // if response is sucessful start read
        {
            for(i=0;i<SD_BLOCK_LEN;i++)
            {
                spi_transfer(0xff);         // write value to buffer pointer
                *buf++=ssp2buf;  
            }

            spi_transfer(0xff);
            spi_transfer(0xff);            // read 16-bit CRC 
        }
        else
        {
            print_UART("Read failed: Invalid token\r\n");
            return;
        }                   
        *token=read;                       // save token
    }
    else
    {
        print_UART("Read failed: Invalid/No response\r\n");
        return;
    }

    spi_transfer(0xff);
    CS_DISABLE();                           // deassert chip select
    spi_transfer(0xff);
}
//------------------------------------------------------------------------------
//                              Write Block
//------------------------------------------------------------------------------
/*******************************************************************************
 Write single 512 byte block
 token = 0x00 - busy timeout
 token = 0x05 - data accepted
 token = 0xff - response timeout
*******************************************************************************/
void SD_writeBlock(char *r1, int addr, char *buf, char *token)
{
    int i=0,j=0;
    char read;
    *token=0xff;                            // Clear token
    spi_transfer(0xff);
    CS_ENABLE();                            // assert chip select
    spi_transfer(0xff);

    SD_command(CMD24, addr, CMD24_CRC);     // send CMD 24 (WRITE_BLOCCK) with starting address
    *r1=SD_readR1();                        // read R1 response

    if(*r1==0x00)                            // continue on CARD - READY
    {
        spi_transfer(SD_START_TOKEN);
        for(i = 0;i<SD_BLOCK_LEN;i++) spi_transfer(buf[i]); // write 512 bytes

        while(i<SD_MAX_WRITE_ATTEMPTS)      // poll for R1 response (timeout = 250ms)
        {
            read=spi_transfer(0xff);
            if (read!=0xff) break;          // exit on valid response                 
            i++;
        }

        // Card needs time to process recieved data - wait for done signal
        if((read & 0x1F)==0x05)             // if data accepted 0bxxx00101
        {
            *token = 0x05;                  // set token to data accepted
            while(spi_transfer(0xff)==0x00) // poll for valid response
            {
                if(i==SD_MAX_WRITE_ATTEMPTS) {*token = 0x00; break;}
                i++;
            }
        }
        else
        {
            print_UART("Write failed: Data not accepted/write attempts exceeded\r\n");
        }

    }
    else
    {
        print_UART("Write failed: Invalid response (CARD NOT READY)\r\n");
    } 

    spi_transfer(0xff);
    CS_DISABLE();                           // deassert chip select
    spi_transfer(0xff);    
}
//------------------------------------------------------------------------------
void signed_write(int p, int q)   // print 4 digit signed number to LCD at row i column j
{
     char thp, hp,tp,op;
     int k=p;
     k=k<<8 | q;

     if(k<0)
     {
        send_data2('-');
        k=-k;
     }
     else
     {
        send_data2('+');
     }

     thp=k/1000;
     hp=(k-thp*1000)/100;
     tp=(k-1000*thp-100*hp)/10;
     op=k-1000*thp-100*hp-10*tp;
     if(k>999)send_data2(thp+48);
     if(k>99)send_data2(hp+48);
     if(k>9)send_data2(tp+48);
     send_data2(op+48);
}
//------------------------------------------------------------------------------