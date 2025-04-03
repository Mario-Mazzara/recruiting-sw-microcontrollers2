#define MOVING_AVG_SIZE 150

extern volatile uint8_t hall_state;

void send_init(UART_HandleTypeDef* _huart,ADC_HandleTypeDef* _hadc);
void send_data(const char *mode);
