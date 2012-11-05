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
volatile int frntdoor_on;
volatile int innrdoor_on;
volatile int buzzer_on;
volatile int light_on;

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
    unsigned char byte = 0;
    byte = (unsigned char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    switch (byte)
    {
        case 'x':
        case 'X':
            // Go into DFU mode
            Jump_To_Bootloader();
            break;
        case 'g':
        case 'G':
            //toggle green led briefly (active low)
            bit_clear(PORTD,BIT(5));
            _delay_ms(500);
            bit_set(PORTD,BIT(5));
            break;
        case 'r':
        case 'R':
            //toggle red led briefly (active low)
            bit_clear(PORTD,BIT(6));
            _delay_ms(500);
            bit_set(PORTD,BIT(6));
            break;
        case 'o':
        case 'O':
            //toggle both leds briefly for orange
            bit_clear(PORTD,BIT(6));
            bit_clear(PORTD,BIT(5));
            _delay_ms(500);
            bit_set(PORTD,BIT(6));
            bit_set(PORTD,BIT(5));
            break;
	case '1':
            //open front door
            bit_set(PORTC,BIT(6));
            frntdoor_on = jiffies;
            break;
	case '2':
            //open inner door
            bit_set(PORTC,BIT(7));
            innrdoor_on = jiffies;
            break;
	case '3':
            //toggle buzzer
            if(bit_get(PORTD,BIT(7)))
            {
                bit_set(PORTD,BIT(7));
		buzzer_on = jiffies;
            }
            else
            {
                bit_clear(PORTD,BIT(7));
		buzzer_on = 0;
            }
            break;
	case '4':
            //toggle light
            if(bit_get(PORTB,BIT(0)))
            {
                bit_set(PORTB,BIT(0));
		light_on = jiffies;
            }
            else
            {
                bit_clear(PORTB,BIT(0));
		light_on = 0;
            }
            break;
        default:
            //Nothing
            break;
    }

    cli();

    //if it has been 50ms since we last had a rfid bit
    if ((jiffies-last_read) > 50){
        flg_readcard = 1;
    }

    //if its 5 seconds since we buzzed the front door open
    if ((frntdoor_on > 0) && (jiffies-frntdoor_on) > 500){
         bit_clear(PORTC,BIT(6));
         frntdoor_on = 0;
    }

    //if its 5 seconds since we buzzed the inner door open
    if ((innrdoor_on > 0) && (jiffies-innrdoor_on) > 500){
         bit_clear(PORTC,BIT(7));
         innrdoor_on = 0;
    }

    //if the doorbell has been pressed for > 10 seconds, silence it
    if ((buzzer_on > 0) && (jiffies-buzzer_on) > 1000){
         bit_clear(PORTD,BIT(7));
         buzzer_on = 0;
    }

    //if the light has been on for > 10 seconds, turn it off
    if ((light_on > 0) && (jiffies-light_on) > 1000){
         bit_clear(PORTB,BIT(0));
         light_on = 0;
    }

    if (bit_count > 0 && (flg_readcard==1 || bit_count >= 56))
    {
	    char buf[17];
        //Write code
        snprintf(buf,17,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        CDC_Device_SendString(&VirtualSerial_CDC_Interface,buf);
        CDC_Device_Flush(&VirtualSerial_CDC_Interface);
        bit_count = 0;
        flg_readcard = 0;
    }
    sei();
}
