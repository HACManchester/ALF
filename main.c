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
#include <stdio.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "bitm.h"
#include "main.h"
#include "timer.h"
#include "rfid.h"
#include "uart.h"
#include "digitalinputs.h"
#include "analoginputs.h"

int main(int argc, const char *argv[])
{
    clock_prescale_set(clock_div_1);
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    timer_init();
    puts("#Timers Initalised");

    Control_Init();
    puts("#Control Initialised");

    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;

    RFID_Init();
    puts("#RFID Initialised");

    Digital_Input_Init();
    puts("#Digital Inputs Initialised");

    Analog_Input_Init();
    puts("#Analog Inputs Initialised");

    sei();
    while (1) {
        Control_Task();
        Control_Doorbell();
        RFID_Task();
        Digital_Input_Task();
        Analog_Input_Task();
    }
}

void Control_Init()
{
    // Set C6 to output for front door
    bit_set(DDRC,BIT(3));
    bit_clear(PORTC, BIT(3));

    // Set C4 to input for doorbell, disable pullups
    bit_clear(DDRC,BIT(4));
    bit_clear(PORTC,BIT(4));

    output_enabled = 0;
    return;
}

void Control_Task()
{
    unsigned char byte = 0;

    if (uart_avail())
    {
	    byte = getchar();
	    switch (byte) {
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
		case 'E':
                    puts("#Door enabled");
                    output_enabled = 1;
		    break;
		case 'e':
                    puts("#Door disabled");
                    output_enabled = 0;
		    break;
		case '1':
		    //open front door
		    if (output_enabled)
                    {
                        puts("#Opening Door");
                        bit_set(PORTC,BIT(3));
		        door_on = jiffies + DOOR_ON_TIME;
                    }
                    else
                    {
                        puts("#Door is disabled, Enable the door output with E before opening it.");
                    }

		    break;
		default:
		    //Nothing
		    printf("%c\n", byte);
        }
    }

    //if its 5 seconds since we buzzed the front door open
    if (door_on < jiffies) {
        bit_clear(PORTC,BIT(3));
        door_on = 0;
    }

    return;
}

void Control_Doorbell(void)
{
    // only check the doorbell every 200 ms
    if ((jiffies-doorbell_last_checked) > 20) {
        doorbell_last_checked = jiffies;

        //doorbell is pressed, send a B every 200ms
        if(bit_get(PINC,BIT(4)) == 0) {
            puts("B");
        }
    }
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




