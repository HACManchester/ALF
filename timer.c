#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "bitm.h"
#include "timer.h"

void timer_init()
{
    TCCR1B |= (1 << WGM12);                // target is OCR1A
    TIMSK1 |= (1 << OCIE1A);               // enable timer1_compA ISR
    OCR1A = TIMER_TGT;                     // target value of timer for 10ms tick
    TCCR1B |= ((1 << CS10) | (1 << CS11)); // set prescale to /64
}

ISR(TIMER1_COMPA_vect)
{
    jiffies++;
}
