/*
 * udp_server.h
 *
 *  Created on: 16 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_
#define MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_

void udp_server_task(void *pvParameters);

float udp_server_temperature(void);
float udp_server_humidity(void);
float udp_server_battery(void);

#endif /* MAIN_SERVICE_UDP_SERVER_UDP_SERVER_H_ */
