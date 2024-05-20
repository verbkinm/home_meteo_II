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

const static char *default_iotv_host = "192.168.40.216";
const static uint16_t default_iotv_tcp_port = 2023;

static char *iotv_host = NULL;
static uint16_t iotv_port = 0;

static const char *TAG = "TCP_CLIENT";
const static char *task_name = "tcp_client_task";

static void tcp_client_run();
static void tcp_client_check_conf_file(void);

static void tcp_client_run()
{
    char *rx_buffer = calloc(1, BUFSIZE);
    if (rx_buffer == NULL)
    {
    	ESP_LOGE(TAG, "tcp_client_run rx_buffer = calloc calloc error");
    	return;
    }

    int addr_family = 0;
    int ip_protocol = 0;
    int sock = -1;

    while (1)
    {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, iotv_host, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(iotv_port);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d = %s", errno, strerror(errno));
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", iotv_host, iotv_port);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d = %s", errno, strerror(errno));
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

            int len = recv(sock, rx_buffer, BUFSIZE - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d = %s", errno, strerror(errno));
                break;
            }
            // Data received
            else
            {
            	iotv_data_recived(rx_buffer, len, sock);
            }

//            taskYIELD();
        }

//        if (sock != -1)
//        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
//        }
    }

    ESP_LOGE(TAG, "Shutting down socket and restarting...");
    shutdown(sock, 0);
    close(sock);

    free(rx_buffer);
}

static void tcp_client_check_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
		return;

	cJSON *iotv = cJSON_CreateObject();
	if (iotv == NULL)
		goto end;

	cJSON_AddItemToObjectCS(root, IOTV_STR, iotv);

	cJSON *url_obj = cJSON_CreateString(default_iotv_host);
	if (url_obj == NULL)
		goto end;

	cJSON_AddItemToObject(iotv, URL_STR, url_obj);

	char buf[6] = {0};
	snprintf(buf, 5, "%d", default_iotv_tcp_port);

	cJSON *port_obj = cJSON_CreateString("2023");
	if (port_obj == NULL)
		goto end;

	cJSON_AddItemToObjectCS(iotv, PORT_STR, port_obj);

	get_iotv_config_value(URL_STR, &url_obj->valuestring);
	get_sntp_config_value(PORT_STR, &port_obj->valuestring);

	FILE *file = fopen(IOTV_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, IOTV_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	end:
	cJSON_Delete(root);
}

void service_tcp_client_read_conf(void)
{
	if (iotv_host != NULL)
		free(iotv_host);

	if (!get_iotv_config_value(URL_STR, &iotv_host))
		iotv_host = (char *)default_iotv_host;

	char *buf_port = NULL;
	if (!get_iotv_config_value(PORT_STR, &buf_port))
		iotv_port = default_iotv_tcp_port;
	else
		iotv_port = strtol(buf_port, NULL, 10);
}

void service_tcp_client_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	printf("%s %s start\n", TAG, task_name);

	tcp_client_check_conf_file();
	service_tcp_client_read_conf();

	for( ;; )
	{
		if (glob_get_status_err())
			break;

		if ((glob_get_status_reg() & STATUS_IP_GOT) && (iotv_get()->state == 1))
			tcp_client_run();

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void service_tcp_client_set_url(const char* url)
{
	set_iotv_config_value(URL_STR, url);
	service_tcp_client_read_conf();
}

void service_tcp_client_set_port(uint16_t port)
{
	char portStr[6] = {0};
	snprintf(portStr, 5, "%hu", port);
	set_iotv_config_value(PORT_STR, portStr);
	service_tcp_client_read_conf();
}

const char *service_tcp_client_get_url(void)
{
	return iotv_host;
}

uint16_t service_tcp_client_get_port(void)
{
	return iotv_port;
}
