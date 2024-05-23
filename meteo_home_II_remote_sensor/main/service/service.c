/*
 * service.c
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */
#include "service.h"

void start_services(void)
{
	xTaskCreate(service_wifi_task, "wifi_service_task", 4096, 0, 10, 0);
	xTaskCreate(service_udp_client_task, "udp_client_service_task", 4096, 0, 10, 0);
}
