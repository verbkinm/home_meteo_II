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

static float temperature = INFINITY;
static float humidity = INFINITY;
static float battery = INFINITY;

float udp_server_temperature(void)
{
	return temperature;
}

float udp_server_humidity(void)
{
	return humidity;
}

float udp_server_battery(void)
{
	return battery;
}

void udp_server_task(void *pvParameters)
{
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
		if (glob_get_status_err())
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
            ESP_LOGI(TAG, "%s", rx_buffer);

            if (len == 8)
            {
            	memcpy(&temperature, rx_buffer, 4);
            	memcpy(&humidity, &rx_buffer[4], 4);
            }

//            int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
//            if (err < 0) {
//                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                break;
//            }
        }

		CLEAN_UP:
        shutdown(listen_sock, 0);
        close(listen_sock);
	}

	printf("%s %s stop\n", TAG, task_name);
	free(rx_buffer);
	vTaskDelete(NULL);
}

