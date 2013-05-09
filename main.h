void Control_Init(void);
void Control_Task(void);
void Control_Input(void);
void Check_Input(uint8_t, uint8_t, uint8_t *, char, char);

uint16_t relay1_on;
uint16_t relay2_on;
uint16_t hc1_on;
uint16_t hc2_on;

uint8_t buzzer_last_state;
uint8_t doorbell_last_state;
uint8_t doorstate_last_state;
uint8_t open_last_state;

char buffer[3];
