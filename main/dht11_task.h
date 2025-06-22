// dht11_task.h

float dht11_get_temperature(void);
float dht11_get_humidity(void);
void dht11_read_task(void *pvParameters);