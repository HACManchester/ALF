#include <avr/io.h>
#include <stdio.h>

#include "bitm.h"
#include "timer.h"
#include "digitalinputs.h"

void Digital_Input_Init(void)
{
	digital_last_check = 0;

	bit_clear(DIGITAL_0_DDR, DIGITAL_0_BIT);
	bit_set(DIGITAL_0_PORT, DIGITAL_0_BIT);

	bit_clear(DIGITAL_1_DDR, DIGITAL_1_BIT);
	bit_set(DIGITAL_1_PORT, DIGITAL_1_BIT);

	digital_0_state = bit_is_set(DIGITAL_0_PIN, DIGITAL_0_BIT);
	digital_1_state = bit_is_set(DIGITAL_1_PIN, DIGITAL_1_BIT);
}

void Digital_Input_Task(void)
{
	if ((digital_last_check+10 <= jiffies) || (digital_last_check+10 >= 65533))
	{
		digital_tmp = bit_is_set(DIGITAL_0_PIN, DIGITAL_0_BIT);
		if (digital_tmp !=  digital_0_state)
		{
			if (digital_tmp == 0)
				puts("D0-0");
			else
				puts("D0-1");

			digital_0_state = digital_tmp;
		}

		digital_tmp = bit_is_set(DIGITAL_1_PIN, DIGITAL_1_BIT);
		if (digital_tmp !=  digital_1_state)
		{
			if (digital_tmp == 0)
				puts("D1-0");
			else
				puts("D1-1");

			digital_1_state = digital_tmp;
		}

		digital_last_check = jiffies;
	}

/*
        if ((digital_last_announce+10000 <= jiffies) || (digital_last_announce+10000 >= 65533))
        {
		if ((jiffies+1000) > 1000)
		{
                	if (digital_0_state == 0)
                	        puts("D0-0");
                	else
                	        puts("D0-1");
                	if (digital_1_state == 0)
                	        puts("D1-0");
                	else
                	        puts("D1-1");
		}
                digital_last_announce = jiffies;
        }
*/
}
