#ifndef _TIMER_H
#define _TIMER_H

#define HZ 100
#define PRESCALE 64
#define TIMER_TGT ( ( ( F_CPU / PRESCALE ) / HZ) - 1)
volatile uint16_t jiffies;
void timer_init(void);

#endif
