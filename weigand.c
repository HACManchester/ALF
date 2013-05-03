#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <LUFA/Version.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
#include "bitm.h"
#include "usb.h"
#include "timer.h"
#include "boot.h"

volatile int bit_count = 0;
volatile unsigned char data[4];
volatile int flg_readcard = 0;
volatile int last_read;

void RFID_Init(void)
{
    //_delay_ms(2000);
    // Set D0 and D1 to inputs, enable pull up and enable their interupts
    // for DATA0/1
    bit_clear(DDRD, BIT(0));
    bit_clear(DDRD, BIT(1));
    bit_set(PORTD,BIT(0));
    bit_set(PORTD,BIT(1));
    bit_set(EIMSK,BIT(INT0));
    bit_set(EIMSK,BIT(INT1));
  
    // Set D2 to input for doorbell  
    bit_clear(DDRD, BIT(2));
    bit_set(PORTD,BIT(2));
    bit_set(EIMSK,BIT(INT2));
   
    // Set D3 to input for door open (internal) 
    bit_clear(DDRD, BIT(3));
    bit_set(PORTD,BIT(3));
    bit_set(EIMSK,BIT(INT3));

    // Set D5 and D6 to outputs for LEDs
    bit_set(DDRD, BIT(5));
    bit_set(DDRD, BIT(6));
    bit_set(PORTD,BIT(5));
    bit_set(PORTD,BIT(6));

    // Set C5 to output for buzzer
    bit_set(DDRC, BIT(5));
    bit_set(PORTC,BIT(5));
}

inline void set_data_bit(volatile unsigned char* data, int byte, int bit, int value)
{
    if (value) {
        bit_set(data[byte],BIT(bit));
    } else {
        bit_clear(data[byte],BIT(bit));
    }
}


inline void set_data_bit_sc(volatile unsigned char* data, int bitcount, int value)
{
    set_data_bit(data, bitcount / 8, (7-bitcount % 8), value);
}

void data0_int(void)
{
    last_read = jiffies;
    set_data_bit_sc(data,bit_count,0);
    bit_count++;
    _delay_us(400); // Delay to remove bounce
}

void data1_int(void)
{
    last_read = jiffies;
    set_data_bit_sc(data,bit_count,1);
    bit_count++;
    _delay_us(400); // Delay to remove bounce
}

void RFID_Task(void)
{
    cli();
    //if it has been 200us since we last had a rfid bit, the card has a short ID number
    if ((jiffies-last_read) > 100) {
        flg_readcard = 1;
    }

    //if we have finished reading the card id (it has stopped sending data or we have received 56 bits)
    if (bit_count > 0 && (flg_readcard==1 || bit_count >= 32)) {
        char buf[12];

        if (bit_count > 1)
        {
            //Write code
            snprintf(buf,12,"%.2x%.2x%.2x%.2xs\r\n", 
                data[3], data[2], data[1], data[0]);
            CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
            CDC_Device_Flush(&VirtualSerial_CDC_Interface);
        }
        bit_count = 0;
        flg_readcard = 0;
        for (int i=0;i<4;i++)
        {
            data[i]=0;
        }
    }
    sei();
}
