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
volatile unsigned char data[7];
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

    // Set D5 and D6 to outputs for LEDs
    bit_set(DDRD, BIT(5));
    bit_set(DDRD, BIT(6));
    bit_set(PORTD,BIT(5));
    bit_set(PORTD,BIT(6));

    // Set C5 to output for buzzer
    bit_set(DDRC, BIT(5));
    bit_set(PORTC,BIT(5));

    // Set C6 to output for front door
    bit_set(DDRC,BIT(6));
    bit_clear(DDRC, BIT(6));

    // Set C7 to output for inner door
    bit_set(DDRC,BIT(7));
    bit_clear(PORTC,BIT(7));

    // Set D7 to output for doorbell buzzer
    bit_set(DDRD,BIT(7));
    bit_clear(PORTD,BIT(7));

    // Set B0 to output for light
    bit_set(DDRB,BIT(0));
    bit_clear(PORTB,BIT(0));
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
    _delay_us(100); // Delay to remove bounce
}

void data1_int(void)
{
    last_read = jiffies;
    set_data_bit_sc(data,bit_count,1);
    bit_count++;
    _delay_us(100); // Delay to remove bounce
}

void RFID_Task(void)
{
    cli();
    //if it has been 200us since we last had a rfid bit, the card has a short ID number
    if ((bit_count > 0) && (jiffies-last_read) > 200) {
        flg_readcard = 1;
    }

    //if we have finished reading the card id (it has stopped sending data or we have received 56 bits)
    if (bit_count > 0 && (flg_readcard==1 || bit_count >= 56)) {
        char buf[17];
        //Write code
        snprintf(buf,17,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
        CDC_Device_Flush(&VirtualSerial_CDC_Interface);
        bit_count = 0;
        flg_readcard = 0;
        for (int i=0; i<7; i++) {
            data[i]=0;
        }
    }
    sei();
}
