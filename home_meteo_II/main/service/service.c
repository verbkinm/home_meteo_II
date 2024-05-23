/*
 * service.c
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */
#include "service.h"

#include "wifi/wifi.h"
#include "sntp/sntp.h"
#include "weather/weather.h"
#include "display/display.h"
#include "bme280/bme280.h"
#include "update/update.h"
#include "ble/ble_client.h"
#include "udp_server/udp_server.h"
#include "tcp_client/tcp_client.h"
#include "iotv.h"

void start_services(void)
{
	xTaskCreate(service_wifi_task, "wifi_service_task", 4096, 0, 10, 0);
	xTaskCreate(service_sntp_task, "sntp_service_task", 4096, 0, 10, 0);
	xTaskCreate(service_weather_task, "weather_service_task", 8192, 0, 10, 0);
	xTaskCreate(service_display_task, "display_service_task", 4096, 0, 10, 0);

	xTaskCreate(service_BME280_task, "BME280_service_task", 4096, 0, 10, 0);
	xTaskCreate(service_update_task, "update_service_task", 4096, 0, 1, 0);

	xTaskCreate(service_udp_server_task, "udp_server_task", 4096, 0, 10, 0);
	xTaskCreate(service_iotv_task, "service_iotv_task", 4096, 0, 10, 0);
	xTaskCreate(service_tcp_client_task, "service_tcp_client_task", 8192, 0, 10, 0);
}
