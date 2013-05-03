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

char bell_buf[3] = "B\r\n";

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
        RFID_Task();
        USB_USBTask();
    }
}

void Control_Init()
{

    // Set C6 to output for door and set high
    bit_set(DDRC,BIT(0));
    bit_set(DDRC, BIT(0));

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
        case '1':
            //open front door
            bit_clear(PORTB,BIT(0));
            frntdoor_on = jiffies + 500;
            break;
        default:
            //Nothing
            break;
    }

    //if its 5 seconds since we buzzed the front door open
    if (frntdoor_on < jiffies) {
        bit_set(PORTB,BIT(0));
        frntdoor_on = 0;
    }


    return;
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

ISR(INT2_vect)
{
    doorbell();
}


