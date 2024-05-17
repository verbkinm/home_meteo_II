/*
 * sntp.h
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */

#ifndef MAIN_SERVICE_SNTP_SNTP_H_
#define MAIN_SERVICE_SNTP_SNTP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
//#include "freertos/semphr.h"

#include "Global_def.h"
#include "json/json_config.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "I2C/DS3231.h"

void service_sntp_task(void *pvParameters);
void service_sntp_read_conf(void);
void service_sntp_obtain_time(void);

const char *service_sntp_utc(void);
const char *service_sntp_url(void);

void service_sntp_update(void);

#endif /* MAIN_SERVICE_SNTP_SNTP_H_ */
