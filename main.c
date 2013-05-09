/*
 * Copyright (C) 2012 Kimball Johnson
 * parts Copyright (C) 2009 Chris McClelland
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#define BUZZER 
#define DOORBELL
#define OPEN
#define DOORSTATE
 
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <LUFA/Version.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>
#include <LUFA/Drivers/USB/USB.h>
#include "bitm.h"
#include "main.h"
#include "usb.h"
#include "timer.h"
#include "boot.h"
#include "rfid.h"

char buffer[3] = "B\r\n";

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
    .Config =
    {
        .ControlInterfaceNumber   = 0,
        .DataINEndpoint           =
        {
            .Address          = CDC_TX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
        },
        .DataOUTEndpoint =
        {
            .Address          = CDC_RX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
        },
        .NotificationEndpoint =
        {
            .Address          = CDC_NOTIFICATION_EPADDR,
            .Size             = CDC_NOTIFICATION_EPSIZE,
            .Banks            = 1,
        },
    },
};


int main(int argc, const char *argv[])
{
    clock_prescale_set(clock_div_1);
    MCUSR &= ~(1 << WDRF);
    wdt_disable();
    timer_init();
    Control_Init();
    RFID_Init();
    USB_Init();
    sei();
    while (1) {
        Control_Task();
        Control_Input();
        RFID_Task();
        USB_USBTask();
    }
}

void Control_Init()
{

    // Set C6 to output for relay 1
    bit_set(DDRC,BIT(6));
    bit_clear(PORTC, BIT(6));

    // Set C7 to output for relay 2
    bit_set(DDRC,BIT(7));
    bit_clear(PORTC,BIT(7));

    // Set D7 to output for hc1
    bit_set(DDRD,BIT(7));
    bit_clear(PORTD,BIT(7));

    // Set B0 to output for hc2
    bit_set(DDRB,BIT(0));
    bit_clear(PORTB,BIT(0));

    // Set C4 to input for buzzer
    bit_clear(DDRC,BIT(4));
    bit_clear(PORTC,BIT(4));

    // Set B7 to input with pullups for doorbell
    bit_clear(DDRB,BIT(7));
    bit_set(PORTB,BIT(7));
    
    // Set B6 to input with pullups for open button
    bit_clear(DDRB,BIT(6));
    bit_set(PORTB,BIT(6));
    
    // Set B5 to input with pullups for open state
    bit_clear(DDRB,BIT(5));
    bit_set(PORTB,BIT(5));

    return;
}

void Control_Task()
{
    unsigned char byte = 0;
    byte = (unsigned char)CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    switch (byte) {
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
            //open relay 1
            bit_set(PORTC,BIT(6));
            relay1_on = jiffies + 50;
            break;
        case '2':
            //open relay 2
            bit_set(PORTC,BIT(7));
            relay2_on = jiffies + 50;
            break;
        case '3':
            //toggle hc1
            if(bit_get(PORTD,BIT(7))) {
                bit_set(PORTD,BIT(7));
                hc1_on = jiffies + 100;
            } else {
                bit_clear(PORTD,BIT(7));
                hc1_on = 0;
            }
            break;
        case '4':
            //toggle hc2
            if(bit_get(PORTB,BIT(0))) {
                bit_set(PORTB,BIT(0));
                hc2_on = jiffies + 100;
            } else {
                bit_clear(PORTB,BIT(0));
                hc2_on = 0;
            }
            break;
        default:
            //Nothing
            break;
    }

    //if its 5 seconds since we buzzed relay 1 open
    if (relay1_on < jiffies) {
        bit_clear(PORTC,BIT(6));
        relay1_on = 0;
    }

    //if its 5 seconds since we buzzed relay 2 open
    if (relay2_on < jiffies) {
        bit_clear(PORTC,BIT(7));
        relay2_on = 0;
    }

    //if the doorbell has been pressed for > 10 seconds, silence it
    if (hc1_on < jiffies) {
        bit_clear(PORTD,BIT(7));
        hc1_on = 0;
    }

    //if the light has been on for > 10 seconds, turn it off
    if (hc2_on < jiffies) {
        bit_clear(PORTB,BIT(0));
        hc2_on = 0;
    }

    return;
}

void Control_Input(void)
{
    #ifdef BUZZER
    Check_Input(PINC, BIT(4), buzzer_last_state, 'b', 'B');
    #endif
    
    #ifdef DOORBELL
    Check_Input(PINB, BIT(7), doorbell_last_state, 'D', 'd');
    #endif
    
    #ifdef OPEN
    Check_Input(PINB, BIT(6), open_last_state, 'O', 'o');
    #endif
    
    #ifdef DOORSTATE
    Check_Input(PINB, BIT(5), doorstate_last_state, 'S', 's');
    #endif
}

void Check_Input(uint8_t pin, uint8_t bit, uint8_t *state, char high, char low)
{
    uint8_t tmp = 0;
    
    //get the input status
    tmp = bit_get(pin, bit);
    
    // if the status has changed
    if(tmp != *state) {
        //wait 20ms as a hacky debounce
        _delay_ms(20);
        
        if (tmp == 0)
        {
            buffer[0] = low;
        }
        else
        {
            buffer[0] = high;
        }
        CDC_Device_SendString(&VirtualSerial_CDC_Interface,buffer);
        CDC_Device_Flush(&VirtualSerial_CDC_Interface);
        
        *state = tmp;
    }
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool success = true;

    success &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

// DATA 0 Int
ISR(INT0_vect)
{
    data0_int();
}

// DATA 1 Int
ISR(INT1_vect)
{
    data1_int();
}




