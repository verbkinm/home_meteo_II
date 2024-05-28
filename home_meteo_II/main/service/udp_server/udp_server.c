/*
 * udp_server.c
 *
 *  Created on: 16 мая 2024 г.
 *      Author: user
 */

#include "udp_server.h"

#include "freertos/FreeRTOS.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <math.h>

#include "Global_def.h"
#include "iotv.h"
#include "Local_Lib/local_lib.h"
#include "Local_Lib/local_lvgl_lib.h"

const static char *TAG = "UDP_SERVER";
const static char *task_name = "udp_server_task";

char *rx_buffer;

static remote_sensor_t sensors[2];

const remote_sensor_t *service_upd_server_get_sensor(uint8_t sensor_number)
{
	int idx = inRange(sensor_number, 0, 1);
	return &sensors[idx];
}

void service_udp_server_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	printf("%s %s start\n", TAG, task_name);

	rx_buffer = calloc(1, BUFSIZE);
	if (rx_buffer == NULL)
	{
		ESP_LOGE(TAG, "udp_server_task rx_buffer calloc error");
		vTaskDelete(NULL);
		return;
	}

	while (true)
	{
		if (glob_get_status_err()
				|| (glob_get_update_reg() & UPDATE_NOW))
			break;

		if (!(glob_get_status_reg() & STATUS_WIFI_AP_START))
		{
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}

		char addr_str[128];
		int addr_family = AF_INET;//(int)pvParameters;
		int ip_protocol = 0;
		struct sockaddr_storage dest_addr;

		if (addr_family == AF_INET)
		{
			struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;

			//			dest_addr_ip4->sin_addr.s_addr = inet_addr("192.168.4.1");
			dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
			dest_addr_ip4->sin_family = AF_INET;
			dest_addr_ip4->sin_port = htons(2022);
			ip_protocol = IPPROTO_IP;
		}

		int listen_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (listen_sock < 0)
		{
			printf("%s Unable to create socket: errno %d\n", TAG, errno);
			vTaskDelete(NULL);
			return;
		}

		int opt = 1;
		setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		printf("%s Socket created\n", TAG);

		int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
		if (err != 0)
		{
			printf("%s Socket unable to bind: errno %d\n", TAG, errno);
			printf("%s IPPROTO: %d\n", TAG, addr_family);
			goto CLEAN_UP;
		}
		printf("%s Socket bound, port %d\n", TAG, 2022);

		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
		socklen_t socklen = sizeof(source_addr);

		ESP_LOGI(TAG, "Waiting for data");

		int len = recvfrom(listen_sock, rx_buffer, BUFSIZE, 0, (struct sockaddr *)&source_addr, &socklen);

		if (len < 0)
		{
			ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
			break;
		}
		else
		{
			// Get the sender's ip address as string
			if (source_addr.ss_family == PF_INET)
				inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);

			rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
			ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);

			if (len == 18)
			{
				uint8_t sensor_number = *(uint8_t *)&rx_buffer[SENSOR_NUMBER];
				if (sensor_number > 2)
				{
					ESP_LOGE(TAG, "unknow sensor number %d", sensor_number);
					goto CLEAN_UP;
				}

				time_t now;
				time(&now);
				localtime_r(&now, &sensors[sensor_number].last_connect);

				sensors[sensor_number].error = *(int8_t *)&rx_buffer[SENSOR_ERROR];

				if (sensors[sensor_number].error != 0)
				{
					sensors[sensor_number].temperature = INFINITY;
					sensors[sensor_number].humidity = INFINITY;
					sensors[sensor_number].pressure = INFINITY;
					sensors[sensor_number].battery = INFINITY;
				}
				else
				{
					sensors[sensor_number].temperature = *(float *)&rx_buffer[SENSOR_TEMPERATURE];
					sensors[sensor_number].humidity = *(float *)&rx_buffer[SENSOR_HUMIDITY];
					sensors[sensor_number].pressure = *(float *)&rx_buffer[SENSORP_PRESSURE];
					sensors[sensor_number].battery = *(float *)&rx_buffer[SENSOR_ACCUM];
				}
			}
		}

		CLEAN_UP:
		shutdown(listen_sock, 0);
		close(listen_sock);
	}

	printf("%s %s stop\n", TAG, task_name);
	free(rx_buffer);
	vTaskDelete(NULL);
}

