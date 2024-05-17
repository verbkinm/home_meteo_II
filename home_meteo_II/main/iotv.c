#include "iotv.h"
#include "I2C/DS3231.h"
#include "TFT_touchscreen/TFT_touch_screen.h"
#include "json/json_config.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "service/sntp/sntp.h"
#include "service/update/update.h"
#include "service/weather/weather.h"

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
		DATA_TYPE_BOOL,			// 14 Выкл/Вкл. станцию wifi. 0 - Выключено
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
		.writeChannelType = readType,
		.state = 0,
		.nameSize = 13,
		.descriptionSize = 13
};

static uint64_t iotv_write_func(char *data, uint64_t size, void *obj);
static bool iotv_have_read_channel(uint8_t ch);
static bool iotv_have_write_channel(uint8_t ch);

static uint64_t iotv_write_func(char *data, uint64_t size, void *obj)
{
	if (obj == NULL || data == NULL)
		return 0;

	int socket = *(int *)obj;

	return send(socket, data, size, 0);
}

static bool iotv_have_read_channel(uint8_t ch)
{

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

esp_err_t iotv_init(void)
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
	}

	iot.state = 1;

	return ESP_OK;
}

void iotv_data_recived(const char *data, int size, int sock)
{
	bool error;
	uint64_t cutDataSize, expectedDataSize;

	last_client_socket = sock;

	if (data == NULL)
		return;

	//страховка
	if ((realBufSize + size) >= BUFSIZE)
	{
		iotv_clear_buf_data();
		printf("%s Buffer overflow\n", TAG);
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


				//!!!



				responseReadData(transmitBuffer, BUFSIZE, &iot, header, iotv_write_func, (void *)&sock);
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
				{
					char *pkgData = ptrReadWrite->data;
					switch (ch)
					{
					// дата/время
					case CH_DATE_TIME:
					{
						DS3231_set_iotv_time();
					}
					break;

					// wifi sta
					case CH_WIFI_STA_ENABLE:
					{
						if ((uint8_t*)pkgData == 0)
						{
							set_wifi_config_value(ON_STR, "0");
							esp_wifi_disconnect();
						}
					}
					break;

					case CH_WIFI_STA_HOSTNAME:
					{
						char *hostname;
						hostname = calloc(1, ptrReadWrite->dataSize + 1);
						if (hostname != NULL)
						{
							if (pkgData != NULL)
								memcpy(hostname, pkgData, ptrReadWrite->dataSize);
							set_wifi_config_value(SSID_STR, hostname);
							free(hostname);
						}
					}
					break;

					case CH_WIFI_STA_PWD:
					{
						char *pwd;
						pwd = calloc(1, ptrReadWrite->dataSize + 1);
						if (pwd != NULL)
						{
							if (pkgData != NULL)
								memcpy(pwd, pkgData, ptrReadWrite->dataSize);
							set_wifi_config_value(PWD_STR, pwd);
							free(pwd);
						}
					}
					break;

					case CH_WIFI_STA_AUTOCON:
					{
						char *autoCon;
						autoCon = calloc(1, ptrReadWrite->dataSize + 1);
						if (autoCon != NULL)
						{
							if (pkgData != NULL)
								memcpy(autoCon, pkgData, ptrReadWrite->dataSize);
							set_wifi_config_value(AUTO_STR, autoCon);
							free(autoCon);
						}
					}
					break;

					// Дисплеей
					case CH_DSIPLAY_ROTATE:
					{
						if (*(uint8_t *)pkgData == 1)
						{
							rotate_display(LV_DISP_ROT_180);
							set_display_config_value(ROTATE_STR, "1");
						}
						else
						{
							rotate_display(LV_DISP_ROT_NONE);
							set_display_config_value(ROTATE_STR, "0");
						}
					}
					break;

					case CH_DSIPLAY_BRIGHTNESS:
					{
						set_display_brightness(*(uint8_t *)pkgData);
						char str[5] = {0};
						snprintf(str, 4, "%d", *(uint8_t *)pkgData);
						set_display_config_value(BRIGHTNESS_STR, str);
					}
					break;

					case CH_DSIPLAY_NIGHT_MODE:
					{
						if (*(uint8_t *)pkgData == 1)
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
						snprintf(str, 4, "%d", *(uint8_t *)pkgData);
						set_display_config_value(BRIGHTNESS_DAY_STR, str);
					}
					break;

					case CH_DSIPLAY_NIGHT_BRIGHTNESS:
					{
						char str[5] = {0};
						snprintf(str, 4, "%d", *(uint8_t *)pkgData);
						set_display_config_value(BRIGHTNESS_NIGHT_STR, str);
					}
					break;

					case CH_DSIPLAY_DAY_START:
					{
						if (ptrReadWrite->dataSize == 8)
						{
							char str[9] = {0}; // 9 dd:dd:dd\0
							if (pkgData != NULL)
								memcpy(str, pkgData, sizeof(str) - 1);
							set_display_config_value(DAY_BEGIN_STR, str);
						}
					}
					break;

					case CH_DSIPLAY_NIGHT_START:
					{
						if (ptrReadWrite->dataSize == 8)
						{
							char str[9] = {0}; // 9 dd:dd:dd\0
							if (pkgData != NULL)
								memcpy(str, pkgData, sizeof(str) - 1);
							set_display_config_value(NIGHT_BEGIN_STR, str);
						}
					}
					break;

					// SNTP
					case CH_SNTP_ENABLE:
					{
						if (*(uint8_t *)pkgData == 1)
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
						if (pkgData != NULL)
						{
							char *utc = calloc(1, ptrReadWrite->dataSize + 1);
							if (utc != NULL)
							{
								memcpy(utc, pkgData, ptrReadWrite->dataSize);
								set_sntp_config_value(UTC_STR, utc);
								free(utc);
								service_sntp_read_conf();
							}
						}
					}
					break;

					case CH_SNTP_URL:
					{
						if (pkgData != NULL)
						{
							char *url = calloc(1, ptrReadWrite->dataSize + 1);
							if (url != NULL)
							{
								memcpy(url, pkgData, ptrReadWrite->dataSize);
								set_sntp_config_value(URL_STR, url);
								free(url);
								service_sntp_read_conf();
							}
						}
					}
					break;

					// UPDATE
					case CH_UPDATE_ENABLE:
					{
						if (*(uint8_t *)pkgData == 1)
						{
							glob_set_bits_update_reg(UPDATE_NOTIFICATION);
							set_update_config_value(ON_STR, "1");
						}
						else
						{
							glob_clear_bits_update_reg(UPDATE_NOTIFICATION);
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
						if (pkgData != NULL)
						{
							char *url = calloc(1, ptrReadWrite->dataSize + 1);
							if (url != NULL)
							{
								memcpy(url, pkgData, ptrReadWrite->dataSize);
								set_update_config_value(URL_STR, url);
								free(url);
								service_update_read_conf();
							}
						}
					}
					break;

					// Метео
					case CH_METEO_ENABLE:
					{
						if (*(uint8_t *)pkgData == 1)
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
						if (pkgData != NULL)
						{
							char *city = calloc(1, ptrReadWrite->dataSize + 1);
							if (city != NULL)
							{
								memcpy(city, pkgData, ptrReadWrite->dataSize);
								set_meteo_config_value(CITY_STR, city);
								free(city);
								service_weather_read_conf();
							}
						}
					}
					break;

					// IOTV сервер
					case CH_IOTV_SERVER_HOSTNAME:
					{
						//!!!
					}
					break;

					case CH_IOTV_SERVER_TCP_PORT:
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
					case CH_EXT_REBOOT:
					{
						esp_restart();
					}
					break;
					case CH_EXT_UPDATE_SNTP:
					{
						service_sntp_update();
					}
					break;

					case CH_EXT_UPDATE_METEO:
					{
						service_weather_update();
					}
					break;
					case CH_EXT_UPDATE_UPDATE:
					{
						glob_set_bits_update_reg(UPDATE_CHECK);
					}
					break;
					default:
						break;
					}
				}
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
