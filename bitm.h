#ifndef _BITM_H
#define _BITM_H
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x)) 

#define delay _delay_ms
#endif

#ifndef _BIT_STRUCTS_H
#define _BIT_STRUCTS_H
struct avr_io {
	int pin;
	int port;
	int ddr;
	int bit; 
};
#endif
