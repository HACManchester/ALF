void Control_Init(void);
void Control_Task(void);
void Control_Doorbell(void);

uint16_t door_on;
uint16_t buzzer_on;

uint16_t doorbell_last_checked;

#define DOOR_ON_TIME 50

uint8_t output_enabled;
