void Control_Init(void);
void Control_Task(void);
void Control_Doorbell(void);

volatile int frntdoor_on;
volatile int innrdoor_on;
volatile int buzzer_on;
volatile int light_on;

volatile int doorbell_last_checked;
