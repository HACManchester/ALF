void Control_Init(void);
void Control_Task(void);
void Control_Doorbell(void);

uint16_t frntdoor_on;
uint16_t innrdoor_on;
uint16_t buzzer_on;
uint16_t light_on;

uint16_t doorbell_last_checked;

char buf[3];
