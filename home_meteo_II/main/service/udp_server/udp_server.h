/*
 * udp_server.h
 *
 *  Created on: 16 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_
#define MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_

#include <stdint.h>
#include <time.h>

struct Sensor {
	struct tm last_connect;
	float temperature,
		humidity,
		pressure,
		battery;

	uint8_t sensor_number;
	int8_t error;
};
typedef struct Sensor remote_sensor_t;

void service_udp_server_task(void *pvParameters);

const remote_sensor_t *service_upd_server_get_sensor(uint8_t sensor_number);

#endif /* MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_ */
