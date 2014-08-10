#include <stdio.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "bitm.h"
#include "timer.h"
#include "rfid.h"

volatile int bit_count = 0;
volatile unsigned char data[7];
volatile int flg_readcard = 0;
volatile int last_read;

void RFID_Init(void)
{
    //_delay_ms(2000);
    // Set D2 and D3 to inputs, enable pull up and enable their interupts
    // for DATA0/1
    bit_clear(DDRD, BIT(2));
    bit_clear(DDRD, BIT(3));
    bit_set(PORTD,BIT(2));
    bit_set(PORTD,BIT(3));
    
    EICRA |= 0x0f;

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

    // Set D7 to low to disable rfid reader
    bit_set(DDRD, BIT(7));
    bit_set(PORTD,BIT(7));
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
    if ((bitcount > 0) || (bitcount < (READER_BITS-1)))
    {
        bitcount--;
    	set_data_bit(data, bitcount / 8, (7-bitcount % 8), value);
    }
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
    if ((bit_count > 0) && (jiffies-last_read) > 80) {
        flg_readcard = 1;
    }

    //if we have finished reading the card id (it has stopped sending data or we have received READER_BITS bits)
    if (bit_count > 0 && (flg_readcard==1 || bit_count >= READER_BITS)) {
	putchar('C');	
	for(int x = ((READER_BITS-2)/8)-1; x >= 0; x--)
            printf("%.2x", data[x]);
        putchar('\n');
        bit_count = 0;
        flg_readcard = 0;
        for (int i=0; i<7; i++) {
            data[i]=0;
        }
    }
    sei();
}
