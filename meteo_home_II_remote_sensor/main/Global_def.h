#ifndef MAIN_GLOBAL_DEF_H_
#define MAIN_GLOBAL_DEF_H_

#include <stdint.h>

// Отложенный запуск сервисов

// UDP
#define UDP_SERVER_ADDR						"192.168.4.1"
#define UDP_SERVER_PORT                     2022
#define BUFSIZE 							2048  // по умолчанию, после компиляции, BUFSIZ = 128

// WIFI
#define WIFI_ATTEMPS_CONN_COUNT				3
#define WIFI_SSID							"AP"
#define WIFI_PASSWORD						"01234567"
//#define WIFI_SSID							"TP-Link_A6BE"
//#define WIFI_PASSWORD						"41706831"

// SENSOR
#define SENSOR								0
#define SENSOR_DATA_SIZE					4

#define SENSOR_ARRAY_SIZE					18
#define SENSOR_NUMBER						0
#define SENSOR_ERROR						1
#define SENSOR_TEMPERATURE					2
#define SENSOR_HUMIDITY						6
#define SENSORP_PRESSURE					10
#define SENSOR_ACCUM						14

#define SENSOR_REMOTE_NUMBER				0

// I2C
#define I2C_MASTER_SCL_IO           		GPIO_NUM_22				   /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           		GPIO_NUM_21				   /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              		0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          		400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   		0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   		0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS      			1000

// Адреса i2c устройств
#define BME280_ADDR                			0x76

// DEEP_SLEEP
#define	SLEEP_SEC							10

// Глобалные флаги статусов
enum STATUS_REG {
	STATUS_WIFI_STA_START 		= 0x0001,
	STATUS_WIFI_STA_CONNECTING 	= 0x0002,
	STATUS_WIFI_STA_CONNECTED 	= 0x0004,
	STATUS_IP_GOT 				= 0x0008,
	STATUS_WIFI_AUTOCONNECT 	= 0x0010,
};

#endif /* MAIN_GLOBAL_DEF_H_ */
