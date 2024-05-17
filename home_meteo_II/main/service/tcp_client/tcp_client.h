/*
 * tcp_client.h
 *
 *  Created on: 17 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SERVICE_TCP_CLIENT_TCP_CLIENT_H_
#define MAIN_SERVICE_TCP_CLIENT_TCP_CLIENT_H_

#include "Global_def.h"
#include "json/json_config.h"
#include "Local_Lib/local_lib.h"
#include "Local_Lib/local_lvgl_lib.h"

void service_tcp_client_task(void *pvParameters);

#endif /* MAIN_SERVICE_TCP_CLIENT_TCP_CLIENT_H_ */
