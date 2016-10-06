#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "analoginputs.h"
#include "bitm.h"

void Analog_Input_Init(void)
{
	// Use AREF for reference voltage, set ADLAR to 0 for ADC in correct place
	ADMUX = 0x00;

	// Set analog prescaler to 128
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// Enable ADC, and interrupts
	ADCSRA|=(1<<ADEN);
}

void Analog_Input_Task(void)
{
	uint16_t tmp;

	if ((analog_last_check+20 <= jiffies) || (analog_last_check+20 >= 65533)) {
		if (analog_current_channel == ANALOG_0_ADC)
		{
			tmp = Analog_Read(ANALOG_1_ADC);
			if (tmp != analog_1_last_value)
			{
				// dont fart around and keep repeating when we have a high or low value
				if (!((tmp > 900 && analog_1_last_value > 900) || (tmp < 100 && analog_1_last_value < 100)))
				{
					printf("A1-%04d\n", tmp);
					analog_1_last_value = tmp;
				}
			}
		}
		else
		{
			tmp = Analog_Read(ANALOG_0_ADC);
			if (tmp != analog_0_last_value)
			{
                                      if (!((tmp > 900 && analog_0_last_value > 900) || (tmp < 100 && analog_0_last_value < 100)))
				{
					printf("A0-%04d\n", tmp);
					analog_0_last_value = tmp;
				}
			}
		}

		analog_last_check = jiffies;
	}
}

uint16_t Analog_Read(uint8_t channel)
{
	uint16_t analog_tmp = 0;
	uint8_t low = 0;
	uint16_t high = 0;

	// store and set channel
	analog_current_channel = channel;
	ADMUX=(ADMUX&0xF0)|channel;

	// start conversion
	ADCSRA |= (1<<ADSC);
	while (bit_is_set(ADCSRA, ADSC)) { };

	low = ADCL;
	high = ADCH;

	analog_tmp = low|(high<<8);
	return analog_tmp;
}
