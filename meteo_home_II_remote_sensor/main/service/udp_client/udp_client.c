/*
 * udp_client.c
 *
 *  Created on: 8 мая 2024 г.
 *      Author: user
 */

#include "udp_client.h"
#include "deep_sleep/deep_sleep.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <esp_log.h>
#include "dht22/dht22.h"
#include "driver/gpio.h"

#include "Global_def.h"
#include "Local_Lib/local_lib.h"
#include "I2C/BME280.h"

static const char *TAG = "UDP_CLIENT";

static void udp_client_send_data();

static void udp_client_send_data()
{
	int addr_family, ip_protocol;

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(UDP_SERVER_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(UDP_SERVER_PORT);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
	if (sock < 0)
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return;
	}
	// Set timeout
	//	struct timeval timeout;
	//	timeout.tv_sec = 10;
	//	timeout.tv_usec = 0;
	//	setsockopt (sock, SOL_SOCKET, SO_REUSEPORT, &timeout, sizeof timeout);

	ESP_LOGI(TAG, "Socket created, sending to %s:%d", UDP_SERVER_ADDR, UDP_SERVER_PORT);

	uint8_t arr[SENSOR_ARRAY_SIZE] = {0};
	arr[SENSOR_NUMBER] = SENSOR;

	//	printf("BME280 Sensor Readings\n" );
	//
	//	struct THP thp, thp_without_calibration;
	//	BME280_readValues(&thp, &thp_without_calibration);
	//
	//	arr[SENSOR_ERROR] = thp.err;
	//
	//	memcpy(&arr[SENSOR_TEMPERATURE], &thp.temperature, SENSOR_DATA_SIZE);
	//	memcpy(&arr[SENSOR_HUMIDITY], &thp.humidity, SENSOR_DATA_SIZE);
	//	memcpy(&arr[SENSORP_PRESSURE], &thp.pressure, SENSOR_DATA_SIZE);
	//	//	memcpy(&arr[SENSOR_ACCUM], &getTemperature(), SENSOR_DATA_SIZE);

	setDHTgpio(GPIO_NUM_4);
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_NUM_2, 1);
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	printf("DHT Sensor Readings\n" );
	int ret = readDHT();

	gpio_set_level(GPIO_NUM_2, 0);

	errorHandler(ret);

	arr[SENSOR_ERROR] = ret;

	float t = getTemperature();
	float h = getHumidity();
	memcpy(&arr[SENSOR_TEMPERATURE], &t, SENSOR_DATA_SIZE);
	memcpy(&arr[SENSOR_HUMIDITY], &h, SENSOR_DATA_SIZE);
	//	memcpy(&arr[SENSORP_PRESSURE], &getPressure(), SENSOR_DATA_SIZE);
	//	memcpy(&arr[SENSOR_ACCUM], &getTemperature(), SENSOR_DATA_SIZE);
	//
	printf("Temperature %.2f degC\n", *(float *)&arr[SENSOR_TEMPERATURE]);
	printf("Humidity %.2f %%\n", *(float *)&arr[SENSOR_HUMIDITY]);
	printf("Pressure %.2f \n", *(float *)&arr[SENSORP_PRESSURE]);
	printf("Battery %.2f %%\n\n", *(float *)&arr[SENSOR_ACCUM]);

	ret = sendto(sock, &arr, sizeof(arr), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (ret < 0)
	{
		ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
		return;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	shutdown(sock, 0);
	close(sock);

	ESP_LOGI(TAG, "Message sent %d bytes", ret);
}
void service_udp_client_task(void *pvParameters)
{
	for( ;; )
	{
		if (glob_get_status_reg() & STATUS_IP_GOT)
		{
			udp_client_send_data();
			//			deep_sleep();
		}
		vTaskDelay(1000 * 1 * 1 / portTICK_PERIOD_MS); // Раз в 1 секунды
	}
	vTaskDelete(NULL);

}
