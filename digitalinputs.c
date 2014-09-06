#include <avr/io.h>
#include <stdio.h>

#include "bitm.h"
#include "timer.h"
#include "uart.h"
#include "digitalinputs.h"

void Digital_Input_Init(void)
{
	digital_next_check = jiffies + 100;

	bit_set(DIGITAL_0_DDR, DIGITAL_0_BIT);
	bit_set(DIGITAL_0_PORT, DIGITAL_0_BIT);

	bit_set(DIGITAL_1_DDR, DIGITAL_1_BIT);
	bit_set(DIGITAL_1_PORT, DIGITAL_1_BIT);

	digital_0_state = bit_is_set(DIGITAL_0_PIN, DIGITAL_0_BIT);
	digital_1_state = bit_is_set(DIGITAL_1_PIN, DIGITAL_1_BIT);
}

void Digital_Input_Task(void)
{
	if (digital_next_check > jiffies)
	{
		digital_tmp = bit_is_set(DIGITAL_0_PIN, DIGITAL_0_BIT);
		if (digital_tmp !=  digital_0_state)
		{
			if (digital_tmp == 0)
				puts("D00");
			else
				puts("D01");

			digital_0_state = digital_tmp;
		}

		digital_tmp = bit_is_set(DIGITAL_1_PIN, DIGITAL_1_BIT);
		if (digital_tmp !=  digital_1_state)
		{
			if (digital_tmp == 0)
				puts("D10");
			else
				puts("D11");

			digital_1_state = digital_tmp;
		}

		digital_next_check = jiffies + 100;
	}
}
