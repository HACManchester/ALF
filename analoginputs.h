#define ANALOG_0_ADC 6
#define ANALOG_1_ADC 7

uint16_t analog_next_check;
uint16_t analog_value;

uint16_t analog_0_last_value;
uint16_t analog_1_last_value;

uint16_t analog_current_channel;

void Analog_Input_Init(void);
void Analog_Input_Task(void);
uint16_t Analog_Read(uint8_t);
