#include "iotv.h"
#include "I2C/DS3231.h"
#include "TFT_touchscreen/TFT_touch_screen.h"
#include "json/json_config.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "service/sntp/sntp.h"
#include "service/update/update.h"
#include "service/weather/weather.h"
#include "service/tcp_client/tcp_client.h"
#include "service/bme280/bme280.h"
#include "service/display/display.h"
#include "TFT_touchscreen/TFT_touch_screen.h"

#include "lwip/sockets.h"
#include <esp_wifi.h>

static uint64_t realBufSize = 0;

static const char *TAG = "iotv";

static char recivedBuffer[BUFSIZE];
static char transmitBuffer[BUFSIZE];

static int last_client_socket = 0;

static uint8_t readType[] = {
		// Локальный датчик ТВД
		DATA_TYPE_FLOAT_32,		// 0 температура
		DATA_TYPE_FLOAT_32,		// 1 влажность
		DATA_TYPE_FLOAT_32,		// 2 давление

		// Внешний датчик ТВД 0
		DATA_TYPE_FLOAT_32,		// 3 температура
		DATA_TYPE_FLOAT_32,		// 4 влажность
		DATA_TYPE_FLOAT_32,		// 5 давление
		DATA_TYPE_FLOAT_32,		// 6 % батарейки питания
		DATA_TYPE_INT_64,		// 7 Время получения последних данных c датчика

		// Внешний датчик ТВД 1
		DATA_TYPE_FLOAT_32,		// 8 температура
		DATA_TYPE_FLOAT_32,		// 9 влажность
		DATA_TYPE_FLOAT_32,		// 10 давление
		DATA_TYPE_FLOAT_32,		// 11 % батарейки питания
		DATA_TYPE_INT_64,		// 12 Время получения последних данных c датчика

		// Время / Дата
		DATA_TYPE_INT_64,		// 13 Дата/время в time_t

		// Wi-Fi станция
		DATA_TYPE_NONE,			// 14 Выкл/Вкл. станцию wifi. 0 - Выключено
		DATA_TYPE_STRING,		// 15 Имя wifi точки
		DATA_TYPE_STRING,		// 16 Пароль от wifi точки
		DATA_TYPE_BOOL,			// 17 Выкл./Вкл. автоподключение.
		DATA_TYPE_STRING,		// 18 Дополнительные данные

		// Дисплей
		DATA_TYPE_INT_8,		// 19 Поворот дисплея на 180 градусов
		DATA_TYPE_INT_16,		// 20 Яркость дисплея
		DATA_TYPE_BOOL,			// 21 Выкл/Вкл. ночной режим
		DATA_TYPE_INT_16,		// 22 Яркость днём при включенном ночном режиме
		DATA_TYPE_INT_16,		// 23 Яркость ночью при включенном ночном режиме
		DATA_TYPE_STRING,		// 24 Время начала дневного интервала
		DATA_TYPE_STRING,		// 25 Время начала ночного интервала

		// SNTP
		DATA_TYPE_BOOL,			// 26 Выкл/Вкл.
		DATA_TYPE_STRING,		// 27 UTC от 0, пример: "UTC-2" — МСК часовой пояс
		DATA_TYPE_STRING,		// 28 Url сервера времени

		// Update
		DATA_TYPE_BOOL,			// 29 Выкл/Вкл.
		DATA_TYPE_INT_64,		// 30 Интервал проверки обновлений в секундах
		DATA_TYPE_STRING,		// 31 Url json файла с информацией об обновлении
		DATA_TYPE_STRING,		// 32 Версия прошивки устройства
		DATA_TYPE_STRING,		// 33 Доступная версия прошивки

		// Метео
		DATA_TYPE_BOOL,			// 34 Выкл/Вкл.
		DATA_TYPE_STRING,		// 35 Наименование города
		DATA_TYPE_FLOAT_32,		// 36 широта
		DATA_TYPE_FLOAT_32,		// 37 долгота

		// IOTV сервер
		DATA_TYPE_STRING,		// 38 Hostname
		DATA_TYPE_INT_64,		// 39 Tcp порт

		// SD
		DATA_TYPE_STRING,		// 40 Дополнительные данные

		// Дополнительные параметры / действия
		DATA_TYPE_INT_8,		// 41 Текущая страница
		DATA_TYPE_NONE,			// 42 Перезагрузить устройство
		DATA_TYPE_NONE,			// 43 Обновить данные sntp
		DATA_TYPE_NONE,			// 44 Обновить данные метео
		DATA_TYPE_NONE,			// 45 Проверить наличие обновлений
};

static uint8_t writeType[] = {
		// Локальный датчик ТВД
		DATA_TYPE_NONE,		// 0 температура
		DATA_TYPE_NONE,		// 1 влажность
		DATA_TYPE_NONE,		// 2 давление

		// Внешний датчик ТВД 0
		DATA_TYPE_NONE,		// 3 температура
		DATA_TYPE_NONE,		// 4 влажность
		DATA_TYPE_NONE,		// 5 давление
		DATA_TYPE_NONE,		// 6 % батарейки питания
		DATA_TYPE_NONE,		// 7 Время получения последних данных c датчика

		// Внешний датчик ТВД 1
		DATA_TYPE_NONE,		// 8 температура
		DATA_TYPE_NONE,		// 9 влажность
		DATA_TYPE_NONE,		// 10 давление
		DATA_TYPE_NONE,		// 11 % батарейки питания
		DATA_TYPE_NONE,		// 12 Время получения последних данных c датчика

		// Время / Дата
		DATA_TYPE_INT_64,		// 13 Дата/время в time_t

		// Wi-Fi станция
		DATA_TYPE_BOOL,			// 14 Выкл/Вкл. станцию wifi. 0 - Выключено
		DATA_TYPE_STRING,		// 15 Имя wifi точки
		DATA_TYPE_STRING,		// 16 Пароль от wifi точки
		DATA_TYPE_BOOL,			// 17 Выкл./Вкл. автоподключение.
		DATA_TYPE_NONE,			// 18 Дополнительные данные

		// Дисплей
		DATA_TYPE_INT_8,		// 19 Поворот дисплея на 180 градусов
		DATA_TYPE_INT_16,		// 20 Яркость дисплея
		DATA_TYPE_BOOL,			// 21 Выкл/Вкл. ночной режим
		DATA_TYPE_INT_16,		// 22 Яркость днём при включенном ночном режиме
		DATA_TYPE_INT_16,		// 23 Яркость ночью при включенном ночном режиме
		DATA_TYPE_STRING,		// 24 Время начала дневного интервала
		DATA_TYPE_STRING,		// 25 Время начала ночного интервала

		// SNTP
		DATA_TYPE_BOOL,			// 26 Выкл/Вкл.
		DATA_TYPE_STRING,		// 27 UTC от 0, пример: "UTC-2" — МСК часовой пояс
		DATA_TYPE_STRING,		// 28 Url сервера времени

		// Update
		DATA_TYPE_BOOL,			// 29 Выкл/Вкл.
		DATA_TYPE_INT_64,		// 30 Интервал проверки обновлений в секундах
		DATA_TYPE_STRING,		// 31 Url json файла с информацией об обновлении
		DATA_TYPE_NONE,			// 32 Версия прошивки устройства
		DATA_TYPE_NONE,			// 33 Доступная версия прошивки

		// Метео
		DATA_TYPE_BOOL,			// 34 Выкл/Вкл.
		DATA_TYPE_STRING,		// 35 Наименование города
		DATA_TYPE_NONE,			// 36 широта
		DATA_TYPE_NONE,			// 37 долгота

		// IOTV сервер
		DATA_TYPE_STRING,		// 38 Hostname
		DATA_TYPE_INT_64,		// 39 Tcp порт

		// SD
		DATA_TYPE_NONE,			// 40 Дополнительные данные

		// Дополнительные параметры / действия
		DATA_TYPE_INT_8,		// 41 Текущая страница
		DATA_TYPE_BOOL,			// 42 Перезагрузить устройство
		DATA_TYPE_BOOL,			// 43 Обновить данные sntp
		DATA_TYPE_BOOL,			// 44 Обновить данные метео
		DATA_TYPE_BOOL,			// 45 Проверить наличие обновлений
};

static struct IOTV_Server_embedded iot = {
		.id = 9,
		.name = "Meteo_Home_II",
		.description = "Meteo_Home_II",
		.numberReadChannel = 46,
		.readChannel = NULL,
		.readChannelType = readType,
		.numberWriteChannel = 46,
		.writeChannelType = writeType,
		.state = 0,
		.nameSize = 13,
		.descriptionSize = 13
};

static esp_err_t iotv_init(void);
static uint64_t iotv_write_func(char *data, uint64_t size, void *obj);
static bool iotv_have_read_channel(uint8_t ch);
static bool iotv_have_write_channel(uint8_t ch);

static void iotv_write_data(uint8_t ch, uint8_t *data, size_t dataSize);
static void iotv_prepare_read_data(uint8_t ch);
static void iotv_read_prepare_json(const char *key, uint8_t ch, bool (*get_json_conf)(const char *, char **));
static void iotv_read_prepare_str(const char *str, uint8_t ch);

static void iotv_read_prepare_json(const char *key, uint8_t ch, bool (*get_json_conf)(const char *, char **))
{
	char *str = NULL;

	if (get_json_conf(key, &str) && str != NULL)
	{
		iotv_read_prepare_str(str, ch);
		free(str);
	}
}

static void iotv_read_prepare_str(const char *str, uint8_t ch)
{
	if (str == NULL)
		return;

	size_t newDataSize = strlen(str);
//	uint8_t *chData = (uint8_t *)iot.readChannel[ch].data;
	iot.readChannel[ch].dataSize = newDataSize;

//	ESP_LOGE(TAG, "iotv_read_prepare_str, ch = %d, str = %s", ch, str);
	if (iot.readChannel[ch].data != NULL)
		free(iot.readChannel[ch].data);

	iot.readChannel[ch].data = malloc(newDataSize);
	memcpy(iot.readChannel[ch].data, str, newDataSize);

//	if (ch == 15)
//	{
//		for (int i = 0; i < newDataSize; ++i)
//			printf("%c", chData[i]);
//		printf("\n");
//	}
}

static void iotv_prepare_read_data(uint8_t ch)
{
	uint8_t *data = (uint8_t *)iot.readChannel[ch].data;
	switch (ch)
	{
	// Локальный датчик температура
	case CH_SENSOR_LOCAL_T:
	case CH_SENSOR_LOCAL_H:
	case CH_SENSOR_LOCAL_P:
	{
		const struct THP *thp = service_BME280_get_value();
		if (ch == CH_SENSOR_LOCAL_T)
			memcpy(data, &thp->temperature, iot.readChannel[ch].dataSize);
		else if (ch == CH_SENSOR_LOCAL_H)
			memcpy(data, &thp->humidity, iot.readChannel[ch].dataSize);
		else if (ch == CH_SENSOR_LOCAL_P)
			memcpy(data, &thp->pressure, iot.readChannel[ch].dataSize);
	}
	break;

	// Внешний датчик 0
	case CH_SENSOR_REMOTE_0_T:
	case CH_SENSOR_REMOTE_0_H:
	case CH_SENSOR_REMOTE_0_P:
	case CH_SENSOR_REMOTE_0_B:
	case CH_SENSOR_REMOTE_0_L:
	{

	}
	break;

	// Внешний датчик 1
	case CH_SENSOR_REMOTE_1_T:
	case CH_SENSOR_REMOTE_1_H:
	case CH_SENSOR_REMOTE_1_P:
	case CH_SENSOR_REMOTE_1_B:
	case CH_SENSOR_REMOTE_1_L:
	{

	}
	break;

	// Время / дата
	case CH_DATE_TIME:
	{
		time_t now;
		time(&now);
		memcpy(data, &now, iot.readChannel[ch].dataSize);
	}
	break;

	// wifi sta
	case CH_WIFI_STA_HOSTNAME:
	{
		iotv_read_prepare_json(SSID_STR, ch, get_wifi_config_value);
	}
	break;
	case CH_WIFI_STA_PWD:
	{
		iotv_read_prepare_json(PWD_STR, ch, get_wifi_config_value);
	}
	break;
	case CH_WIFI_STA_AUTOCON:
	{
		if (glob_get_status_reg() & STATUS_WIFI_AUTOCONNECT)
			*(bool *)data = true;
		else
			*(bool *)data = false;
	}
	break;
	case CH_WIFI_STA_META:
	{
		//!!!
	}
	break;

	// Дисплей
	case CH_DSIPLAY_ROTATE:
	{
		*(int8_t *)data = (int8_t)TFT_get_display_rotate();
	}
	break;
	case CH_DSIPLAY_BRIGHTNESS:
	{
		*(int16_t *)data = (int16_t)service_display_conf()->brightnes;
	}
	break;
	case CH_DSIPLAY_NIGHT_MODE:
	{
		if (glob_get_status_reg() & STATUS_DISPLAY_NIGHT_MODE_ON)
			*(bool *)data = true;
		else
			*(bool *)data = false;
	}
	break;
	case CH_DSIPLAY_DAY_BRIGHTNESS:
	{
		*(int16_t *)data = (int16_t)service_display_conf()->brightness_day;
	}
	break;

	case CH_DSIPLAY_NIGHT_BRIGHTNESS:
	{
		*(int16_t *)data = (int16_t)service_display_conf()->brightness_night;
	}
	break;
	case CH_DSIPLAY_DAY_START:
	{
		iotv_read_prepare_json(DAY_BEGIN_STR, ch, get_display_config_value);
	}
	break;
	case CH_DSIPLAY_NIGHT_START:
	{
		iotv_read_prepare_json(NIGHT_BEGIN_STR, ch, get_display_config_value);
	}
	break;

	// SNTP
	case CH_SNTP_ENABLE:
	{
		if (glob_get_status_reg() & STATUS_SNTP_ON)
			*(bool *)data = true;
		else
			*(bool *)data = false;
	}
	break;
	case CH_SNTP_UCT:
	{
		iotv_read_prepare_json(UTC_STR, ch, get_sntp_config_value);
	}
	break;

	case CH_SNTP_URL:
	{
		iotv_read_prepare_json(URL_STR, ch, get_sntp_config_value);
	}
	break;

	// UPDATE
	case CH_UPDATE_ENABLE:
	{
		if (glob_get_status_reg() & UPDATE_ON)
			*(bool *)data = true;
		else
			*(bool *)data = false;
	}
	break;
	case CH_UPDATE_INTERVAL:
	{

	}
	break;
	case CH_UPDATE_URL:
	{
		iotv_read_prepare_json(URL_STR, ch, get_update_config_value);
	}
	break;
	case CH_UPDATE_VERSION:
	{
		iotv_read_prepare_str(esp_app_get_description()->version, ch);
	}
	break;
	case CH_UPDATE_NEW_VERSION:
	{
		iotv_read_prepare_str(service_update_get_available_version(), ch);
	}
	break;

	// Метео
	case CH_METEO_ENABLE:
	{
		if (glob_get_status_reg() & STATUS_METEO_ON)
			*(bool *)data = true;
		else
			*(bool *)data = false;
	}
	break;
	case CH_METEO_CITY:
	{
		iotv_read_prepare_str(service_weather_get_city(), ch);
	}
	break;
	case CH_METEO_LATITUDE:
	{
		//!!!
	}
	break;
	case CH_METEO_LONGITUDE:
	{
		//!!!
	}
	break;

	// IOTV сервер
	case CH_IOTV_SERVER_HOSTNAME:
	{
		iotv_read_prepare_str(service_tcp_client_get_url(), ch);
	}
	break;
	case CH_IOTV_SERVER_TCP_PORT:
	{
		*(uint64_t *)data = service_tcp_client_get_port();
	}
	break;

	// SD
	case CH_SD_META:
	{
		//!!!
	}
	break;

	// Дополнительные параметры / действия
	case CH_EXT_CUR_PAGE:
	{
		//!!!
	}
	break;
	default:
		break;
	}
}

static void iotv_write_data(uint8_t ch, uint8_t *data, size_t dataSize)
{
	if (data == NULL || dataSize == 0)
		return;

	switch (ch)
	{
	// дата/время
	case CH_DATE_TIME:
	{
		//		DS3231_set_iotv_time();
	}
	break;

	// wifi sta
	case CH_WIFI_STA_ENABLE:
	{
		if (*(uint8_t*)data == 0)
		{
			set_wifi_config_value(ON_STR, "0");
			esp_wifi_disconnect();
		}
	}
	break;
	case CH_WIFI_STA_HOSTNAME:
	{
		char *hostname;
		hostname = calloc(1, dataSize + 1);
		if (hostname != NULL)
		{
			memcpy(hostname, data, dataSize);
			set_wifi_config_value(SSID_STR, hostname);
			free(hostname);
		}
	}
	break;
	case CH_WIFI_STA_PWD:
	{
		char *pwd;
		pwd = calloc(1, dataSize + 1);
		if (pwd != NULL)
		{
			memcpy(pwd, data, dataSize);
			set_wifi_config_value(PWD_STR, pwd);
			free(pwd);
		}
	}
	break;
	case CH_WIFI_STA_AUTOCON:
	{
		if (*(uint8_t*)data == 0)
		{
			set_wifi_config_value(AUTO_STR, "0");
			glob_clear_bits_status_reg(STATUS_WIFI_AUTOCONNECT);
		}
		else
		{
			set_wifi_config_value(AUTO_STR, "1");
			glob_set_bits_status_reg(STATUS_WIFI_AUTOCONNECT);
		}
	}
	break;

	// Дисплеей
	case CH_DSIPLAY_ROTATE:
	{
		if (*(uint8_t *)data == 1)
		{
			TFT_set_display_rotate(LV_DISP_ROT_180);
			set_display_config_value(ROTATE_STR, "1");
		}
		else
		{
			TFT_set_display_rotate(LV_DISP_ROT_NONE);
			set_display_config_value(ROTATE_STR, "0");
		}
	}
	break;
	case CH_DSIPLAY_BRIGHTNESS:
	{
		TFT_set_display_brightness(*(uint8_t *)data);
		char str[5] = {0};
		snprintf(str, 4, "%d", *(uint8_t *)data);
		set_display_config_value(BRIGHTNESS_STR, str);
	}
	break;
	case CH_DSIPLAY_NIGHT_MODE:
	{
		if (*(uint8_t *)data == 1)
		{
			set_display_config_value(NIGHT_MODE_STR, "1");
			glob_set_bits_status_reg(STATUS_DISPLAY_NIGHT_MODE_ON);
		}
		else
		{
			set_display_config_value(NIGHT_MODE_STR, "0");
			glob_clear_bits_status_reg(STATUS_DISPLAY_NIGHT_MODE_ON);
		}
	}
	break;
	case CH_DSIPLAY_DAY_BRIGHTNESS:
	{
		char str[5] = {0};
		snprintf(str, 4, "%d", *(uint8_t *)data);
		set_display_config_value(BRIGHTNESS_DAY_STR, str);
	}
	break;
	case CH_DSIPLAY_NIGHT_BRIGHTNESS:
	{
		char str[5] = {0};
		snprintf(str, 4, "%d", *(uint8_t *)data);
		set_display_config_value(BRIGHTNESS_NIGHT_STR, str);
	}
	break;
	case CH_DSIPLAY_DAY_START:
	{
		if (dataSize == 8)
		{
			char str[9] = {0}; // 9 dd:dd:dd\0
			memcpy(str, data, sizeof(str) - 1);
			set_display_config_value(DAY_BEGIN_STR, str);
		}
	}
	break;
	case CH_DSIPLAY_NIGHT_START:
	{
		if (dataSize == 8)
		{
			char str[9] = {0}; // 9 dd:dd:dd\0
			memcpy(str, data, sizeof(str) - 1);
			set_display_config_value(NIGHT_BEGIN_STR, str);
		}
	}
	break;

	// SNTP
	case CH_SNTP_ENABLE:
	{
		if (*(uint8_t *)data == 1)
		{
			glob_set_bits_status_reg(STATUS_SNTP_ON);
			set_sntp_config_value(ON_STR, "1");
		}
		else
		{
			glob_clear_bits_status_reg(STATUS_SNTP_ON);
			set_sntp_config_value(ON_STR, "0");
		}
	}
	break;
	case CH_SNTP_UCT:
	{
		char *utc = calloc(1, dataSize + 1);
		if (utc != NULL)
		{
			memcpy(utc, data, dataSize);
			set_sntp_config_value(UTC_STR, utc);
			free(utc);
			service_sntp_read_conf();
		}
	}
	break;
	case CH_SNTP_URL:
	{
		char *url = calloc(1, dataSize + 1);
		if (url != NULL)
		{
			memcpy(url, data, dataSize);
			set_sntp_config_value(URL_STR, url);
			free(url);
			service_sntp_read_conf();
		}
	}
	break;

	// UPDATE
	case CH_UPDATE_ENABLE:
	{
		if (*(uint8_t *)data == 1)
		{
			glob_set_bits_update_reg(UPDATE_ON);
			set_update_config_value(ON_STR, "1");
		}
		else
		{
			glob_clear_bits_update_reg(UPDATE_ON);
			set_update_config_value(ON_STR, "0");
		}
	}
	break;
	case CH_UPDATE_INTERVAL:
	{

	}
	break;
	case CH_UPDATE_URL:
	{
		char *url = calloc(1, dataSize + 1);
		if (url != NULL)
		{
			memcpy(url, data, dataSize);
			set_update_config_value(URL_STR, url);
			free(url);
			service_update_read_conf();
		}
	}
	break;

	// Метео
	case CH_METEO_ENABLE:
	{
		if (*(uint8_t *)data == 1)
		{
			glob_set_bits_status_reg(STATUS_METEO_ON);
			set_meteo_config_value(ON_STR, "1");
		}
		else
		{
			glob_clear_bits_status_reg(STATUS_METEO_ON);
			set_meteo_config_value(ON_STR, "0");
		}
	}
	break;
	case CH_METEO_CITY:
	{
		char *city = calloc(1, dataSize + 1);
		if (city != NULL)
		{
			memcpy(city, data, dataSize);
			set_meteo_config_value(CITY_STR, city);
			free(city);
			service_weather_read_conf();
		}
	}
	break;

	// IOTV сервер
	case CH_IOTV_SERVER_HOSTNAME:
	{
		char *url = calloc(1, dataSize + 1);
		if (url != NULL)
		{
			memcpy(url, data, dataSize);
			service_tcp_client_set_url(url);
			free(url);
		}
	}
	break;
	case CH_IOTV_SERVER_TCP_PORT:
	{
		service_tcp_client_set_port(*(uint64_t *)data);
	}
	break;

	// Дополнительные параметры / действия
	case CH_EXT_CUR_PAGE:
	{
		//!!!
	}
	break;
	case CH_EXT_REBOOT:
	{
		if (*(uint8_t *)data == 1)
			esp_restart();
	}
	break;
	case CH_EXT_UPDATE_SNTP:
	{
		if (*(uint8_t *)data == 1)
			service_sntp_update();
	}
	break;
	case CH_EXT_UPDATE_METEO:
	{
		if (*(uint8_t *)data == 1)
			service_weather_update();
	}
	break;
	case CH_EXT_UPDATE_UPDATE:
	{
		if (*(uint8_t *)data == 1)
			glob_set_bits_update_reg(UPDATE_CHECK);
	}
	break;
	default:
		break;
	}
}

static uint64_t iotv_write_func(char *data, uint64_t size, void *obj)
{
	if (obj == NULL || data == NULL)
		return 0;

	int socket = *(int *)obj;

	//	printf("data -> ");
	//	for (int i = 0; i < size; ++i)
	//	{
	//		printf("%02X:", data[i]);
	//	}
	//	printf("\n");

	return send(socket, data, size, 0);
}

static bool iotv_have_read_channel(uint8_t ch)
{
	switch(ch)
	{
	case CH_WIFI_STA_ENABLE:
	case CH_EXT_REBOOT:
	case CH_EXT_UPDATE_SNTP:
	case CH_EXT_UPDATE_METEO:
	case CH_EXT_UPDATE_UPDATE:
		return false;
	}

	return true;
}

static bool iotv_have_write_channel(uint8_t ch)
{
	switch(ch)
	{
	case  CH_SENSOR_LOCAL_T:
	case  CH_SENSOR_LOCAL_H:
	case  CH_SENSOR_LOCAL_P:

	case CH_SENSOR_REMOTE_0_T:
	case CH_SENSOR_REMOTE_0_H:
	case CH_SENSOR_REMOTE_0_P:
	case CH_SENSOR_REMOTE_0_B:
	case CH_SENSOR_REMOTE_0_L:

	case CH_SENSOR_REMOTE_1_T:
	case CH_SENSOR_REMOTE_1_H:
	case CH_SENSOR_REMOTE_1_P:
	case CH_SENSOR_REMOTE_1_B:
	case CH_SENSOR_REMOTE_1_L:

	case CH_WIFI_STA_META:

	case CH_UPDATE_VERSION:
	case CH_UPDATE_NEW_VERSION:

	case CH_METEO_LATITUDE:
	case CH_METEO_LONGITUDE:

	case CH_SD_META:
		return false;
	}

	return true;
}

void iotv_clear_buf_data(void)
{
	realBufSize = 0;
	last_client_socket = 0;
}

const struct IOTV_Server_embedded *iotv_get(void)
{
	return &iot;
}

static esp_err_t iotv_init(void)
{
	// Выделения памяти для iot структуры
	iot.readChannel = (struct RawEmbedded *)malloc(sizeof(struct RawEmbedded) * iot.numberReadChannel);

	if (iot.readChannel == NULL)
	{
		ESP_LOGE(TAG, "iotv_init iot.readChannel - calloc error");
		return ESP_FAIL;
	}

	for (uint8_t i = 0; i < iot.numberReadChannel; ++i)
	{
		iot.readChannel[i].dataSize = dataSizeonDataType(readType[i]);

		if (iot.readChannel[i].dataSize > 0)
		{
			iot.readChannel[i].data = (char *)calloc(1, iot.readChannel[i].dataSize);
			if (iot.readChannel[i].data == NULL)
			{
				ESP_LOGE(TAG, "iotv_init iot.readChannel[i].data - calloc error");
				return ESP_FAIL;
			}
		}
		else
			iot.readChannel[i].data = NULL;
	}

	iot.state = 1;

	return ESP_OK;
}

void iotv_data_recived(const char *data, int size, int sock)
{
	//	printf("data <- ");
	//	for (int i = 0; i < size; ++i)
	//	{
	//		printf("%02X:", data[i]);
	//	}
	//	printf("\n");

	bool error;
	uint64_t cutDataSize, expectedDataSize;

	last_client_socket = sock;

	if (data == NULL)
		return;

	//страховка
	if ((realBufSize + size) >= BUFSIZE)
	{
		iotv_clear_buf_data();
		printf("%s Buffer overflow. Size = %llu\n", TAG, realBufSize + size);
		return;
	}

	memcpy(&recivedBuffer[realBufSize], data, size);
	realBufSize += size;

	while (realBufSize > 0)
	{
		uint64_t size = 0;

		struct Header* header = createPkgs((uint8_t *)recivedBuffer, realBufSize, &error, &expectedDataSize, &cutDataSize);

		if (header == NULL)
			printf("%s header == NULL\n", TAG);

		if (error == true)
		{
			iotv_clear_buf_data();
			printf("%s Data error\n", TAG);
			break;
		}

		if (expectedDataSize > 0)
		{
			printf("%s expextedDataSize %d\n", TAG, (int)expectedDataSize);
			return;
		}

		if (header->type == HEADER_TYPE_REQUEST)
		{
			if (header->assignment == HEADER_ASSIGNMENT_IDENTIFICATION)
				size = responseIdentificationData(transmitBuffer, BUFSIZE, &iot, 0);
			else if(header->assignment == HEADER_ASSIGNMENT_READ)
			{
				responseReadData(transmitBuffer, BUFSIZE, &iot, header, iotv_write_func, (void *)&sock, ReadWrite_FLAGS_NONE, HEADER_FLAGS_NONE);
				size = 0;
			}
			else if (header->assignment == HEADER_ASSIGNMENT_WRITE)
			{
				struct Read_Write *ptrReadWrite = ((struct Read_Write *)header->pkg);
				uint8_t ch = ptrReadWrite->channelNumber;

				if (iotv_have_write_channel(ch) == false) // Отсутствие каналов записи
				{
					ptrReadWrite->dataSize = 0; // что бы не произошло изменение в iot при вызове функции responseWriteData
					responseWriteData((char *)transmitBuffer, BUFSIZE, &iot, header, ReadWrite_FLAGS_ERROR, 0);
				}
				else
					size = responseWriteData((char *)transmitBuffer, BUFSIZE, &iot, header, 0, 0);

				if (size > 0)
					iotv_write_data(ch, (uint8_t *)ptrReadWrite->data, ptrReadWrite->dataSize);
			}
			else if(header->assignment == HEADER_ASSIGNMENT_PING_PONG)
				size = responsePingData(transmitBuffer, BUFSIZE);
			else if(header->assignment == HEADER_ASSIGNMENT_STATE)
				size = responseStateData(transmitBuffer, BUFSIZE, &iot);
		}

		if (size)
			iotv_write_func((char *)transmitBuffer, size, (void *)&sock);

		memcpy(recivedBuffer, &recivedBuffer[cutDataSize], BUFSIZE - cutDataSize);
		realBufSize -= cutDataSize;

		clearHeader(header);
	}
}

void service_iotv_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);

	if (iotv_init() == ESP_FAIL)
		ESP_LOGE(TAG, "iotv init fail");

	for( ;; )
	{
		if (glob_get_status_err() || (glob_get_update_reg() & UPDATE_NOW))
			break;

		if ((glob_get_status_reg() & STATUS_IP_GOT) && (iotv_get()->state == 1))
		{
			for (int i = 0; i < iot.numberReadChannel; ++i)
				iotv_prepare_read_data(i);
		}

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

