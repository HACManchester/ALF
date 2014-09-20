#define DIGITAL_0_PORT PORTB
#define DIGITAL_0_PIN PINB
#define DIGITAL_0_DDR DDRB
#define DIGITAL_0_BIT _BV(0)

#define DIGITAL_1_PORT PORTB
#define DIGITAL_1_PIN PINB
#define DIGITAL_1_DDR DDRB
#define DIGITAL_1_BIT _BV(1)

uint16_t digital_next_check;
uint16_t digital_next_announce;
uint8_t digital_0_state;
uint8_t digital_1_state;
uint8_t digital_tmp;

void Digital_Input_Init(void);
void Digital_Input_Task(void);
