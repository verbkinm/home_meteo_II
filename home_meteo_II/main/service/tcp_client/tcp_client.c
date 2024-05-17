/*
 * tcp_client.c
 *
 *  Created on: 17 мая 2024 г.
 *      Author: user
 */

#include "tcp_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"

#include "iotv.h"

#define HOST_IP_ADDR		"192.168.0.1"
#define TCP_PORT			2022

static const char *TAG = "TCP_CLIENT";
const static char *task_name = "tcp_client_task";

static void tcp_client_run();

static void tcp_client_run()
{
    char *rx_buffer = calloc(1, BUFSIZE);
    if (rx_buffer == NULL)
    {
    	ESP_LOGE(TAG, "tcp_client_run rx_buffer = calloc calloc error");
    	return;
    }

    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1)
        {
//            int err = send(sock, payload, strlen(payload), 0);
//            if (err < 0) {
//                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                break;
//            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
            	iotv_data_recived(rx_buffer, BUFSIZE, sock);
            }

            taskYIELD();
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    free(rx_buffer);
}

void service_tcp_client_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	printf("%s %s start\n", TAG, task_name);

	for( ;; )
	{
		if (glob_get_status_err())
			break;

		if (glob_get_status_reg() & STATUS_WIFI_AP_START)
			tcp_client_run();

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
