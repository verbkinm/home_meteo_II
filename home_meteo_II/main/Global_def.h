#ifndef MAIN_GLOBAL_DEF_H_
#define MAIN_GLOBAL_DEF_H_

// Отложенный запуск сервисов

#define DELAYED_LAUNCH						0//10000

// SENSOR

#define SENSOR_COUNT						2
#define SENSOR_DATA_SIZE					4

#define SENSOR_ARRAY_SIZE					18
#define SENSOR_NUMBER						0
#define SENSOR_ERROR						1
#define SENSOR_TEMPERATURE					2
#define SENSOR_HUMIDITY						6
#define SENSORP_PRESSURE					10
#define SENSOR_ACCUM						14

// IOTV

#define CH_SENSOR_LOCAL_T					0
#define CH_SENSOR_LOCAL_H					1
#define CH_SENSOR_LOCAL_P					2

#define CH_SENSOR_REMOTE_0_T				3
#define CH_SENSOR_REMOTE_0_H				4
#define CH_SENSOR_REMOTE_0_P				5
#define CH_SENSOR_REMOTE_0_B				6
#define CH_SENSOR_REMOTE_0_L				7

#define CH_SENSOR_REMOTE_1_T				8
#define CH_SENSOR_REMOTE_1_H				9
#define CH_SENSOR_REMOTE_1_P				10
#define CH_SENSOR_REMOTE_1_B				11
#define CH_SENSOR_REMOTE_1_L				12

#define CH_DATE_TIME						13

#define CH_WIFI_STA_ENABLE					14
#define CH_WIFI_STA_HOSTNAME				15
#define CH_WIFI_STA_PWD						16
#define CH_WIFI_STA_AUTOCON					17
#define CH_WIFI_STA_META					18

#define CH_DSIPLAY_ROTATE					19
#define CH_DSIPLAY_BRIGHTNESS				20
#define CH_DSIPLAY_NIGHT_MODE				21
#define CH_DSIPLAY_DAY_BRIGHTNESS			22
#define CH_DSIPLAY_NIGHT_BRIGHTNESS			23
#define CH_DSIPLAY_DAY_START				24
#define CH_DSIPLAY_NIGHT_START				25

#define CH_SNTP_ENABLE						26
#define CH_SNTP_UCT							27
#define CH_SNTP_URL							28

#define CH_UPDATE_ENABLE					29
#define CH_UPDATE_INTERVAL					30
#define CH_UPDATE_URL						31
#define CH_UPDATE_VERSION					32
#define CH_UPDATE_NEW_VERSION				33

#define CH_METEO_ENABLE						34
#define CH_METEO_CITY						35
#define CH_METEO_LATITUDE					36
#define CH_METEO_LONGITUDE					37

#define CH_IOTV_SERVER_HOSTNAME				38
#define CH_IOTV_SERVER_TCP_PORT				39

#define CH_SD_META							40

#define CH_EXT_CUR_PAGE						41
#define CH_EXT_REBOOT						42
#define CH_EXT_UPDATE_SNTP					43
#define CH_EXT_UPDATE_METEO					44
#define CH_EXT_UPDATE_UPDATE				45

// TCP

#define PORT                        		8888
#define KEEPALIVE_IDLE              		5
#define KEEPALIVE_INTERVAL          		5
#define KEEPALIVE_COUNT             		3
#define BUFSIZE 							2048  // по умолчанию, после компиляции, BUFSIZ = 128
#define HOSTNAME							"meteo_home"

// I2C

#define I2C_MASTER_SCL_IO           		GPIO_NUM_20				   /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           		GPIO_NUM_19				   /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              		0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          		400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   		0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   		0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS      			1000

// Адреса i2c устройств

#define BME280_ADDR                			0x76
#define DS3231_ADDR							0x68

// SPI

#define	SPI_NUM_CS							10
#define	SPI_NUM_MOSI						11
#define	SPI_NUM_MISO						13
#define	SPI_NUM_SCK							12

// TFT

#define LCD_PIXEL_CLOCK_HZ     				(18 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL 				1
#define LCD_BK_LIGHT_OFF_LEVEL 				!LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_BK_LIGHT       				GPIO_NUM_2
#define PIN_NUM_HSYNC          				GPIO_NUM_39
#define PIN_NUM_VSYNC          				GPIO_NUM_41
#define PIN_NUM_DE             				GPIO_NUM_40
#define PIN_NUM_PCLK           				GPIO_NUM_42
#define PIN_NUM_DATA0          				GPIO_NUM_8  // B0
#define PIN_NUM_DATA1          				GPIO_NUM_3  // B1
#define PIN_NUM_DATA2          				GPIO_NUM_46 // B2
#define PIN_NUM_DATA3          				GPIO_NUM_9  // B3
#define PIN_NUM_DATA4          				GPIO_NUM_1  // B4
#define PIN_NUM_DATA5          				GPIO_NUM_5  // G0
#define PIN_NUM_DATA6          				GPIO_NUM_6  // G1
#define PIN_NUM_DATA7        				GPIO_NUM_7  // G2
#define PIN_NUM_DATA8          				GPIO_NUM_15 // G3
#define PIN_NUM_DATA9          				GPIO_NUM_16 // G4
#define PIN_NUM_DATA10         				GPIO_NUM_4  // G5
#define PIN_NUM_DATA11         				GPIO_NUM_45 // R0
#define PIN_NUM_DATA12        				GPIO_NUM_48 // R1
#define PIN_NUM_DATA13         				GPIO_NUM_47 // R2
#define PIN_NUM_DATA14         				GPIO_NUM_21 // R3
#define PIN_NUM_DATA15         				GPIO_NUM_14 // R4
#define PIN_NUM_DISP_EN        				GPIO_NUM_NC
#define PIN_TOUCH_RST						GPIO_NUM_38

// The pixel number in horizontal and vertical
#define LCD_H_RES              				800
#define LCD_V_RES              				480

#define LCD_NUM_FB             				1

#define LVGL_TICK_PERIOD_MS    				5
#define LCD_PANEL_STATUS_H	   				42

// Для регулировки яркости дисплея

#define LEDC_TIMER             				LEDC_TIMER_0
#define LEDC_MODE              				LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          			PIN_NUM_BK_LIGHT
#define LEDC_CHANNEL            			LEDC_CHANNEL_0
#define LEDC_DUTY_RES           			LEDC_TIMER_8_BIT
#define LEDC_DUTY               			(64) // Set duty to 50%. ((2 ** 8) - 1) * 50%
#define LEDC_FREQUENCY          			(120) // Frequency in Hertz.

// FS PATH

#define	SD_ON				"A:/sdcard/panel/sd_on.png"
#define	SD_OFF				"A:/sdcard/panel/sd_off.png"
#define	WIFI_CONNECTED		"A:/sdcard/panel/wifi_con.png"
#define	WIFI_CONNECTING		"A:/sdcard/panel/wifi_ing.png"
#define	WIFI_DISABLE		"A:/sdcard/panel/wifi_off.png"
#define	WIFI_ENABLE			"A:/sdcard/panel/wifi_on.png"

#define TEMPERATURE			"A:/sdcard/img/t.png"
#define HUMIDITY			"A:/sdcard/img/h.png"
#define PRESSURE			"A:/sdcard/img/p.png"
#define TEMPERATURE32		"A:/sdcard/img/t32.png"
#define HUMIDITY32			"A:/sdcard/img/h32.png"
#define PRESSURE32			"A:/sdcard/img/p32.png"

#define CLOUD_COVER_1		"A:/sdcard/img/c_1.png"
#define CLOUD_COVER_2		"A:/sdcard/img/c_2.png"
#define CLOUD_COVER_3		"A:/sdcard/img/c_3.png"
#define CLOUD_COVER_4		"A:/sdcard/img/c_4.png"

#define RAIN_1				"A:/sdcard/img/r_1.png"
#define RAIN_2				"A:/sdcard/img/r_2.png"
#define RAIN_3				"A:/sdcard/img/r_3.png"
#define RAIN_4				"A:/sdcard/img/r_4.png"

#define SNOW_1				"A:/sdcard/img/s_1.png"
#define SNOW_2				"A:/sdcard/img/s_2.png"
#define SNOW_3				"A:/sdcard/img/s_3.png"
#define SNOW_4				"A:/sdcard/img/s_4.png"

#define SNOW_AND_RAIN		"A:/sdcard/img/sr.png"
#define PRECIPITATION0		"A:/sdcard/img/sr0.png"

#define WIND				"A:/sdcard/img/w.png"
#define WIND_NO				"A:/sdcard/img/w_no.png"
#define WIND_0				"A:/sdcard/img/w0.png"
#define WIND_45				"A:/sdcard/img/w45.png"
#define WIND_90				"A:/sdcard/img/w90.png"
#define WIND_135			"A:/sdcard/img/w135.png"
#define WIND_180			"A:/sdcard/img/w180.png"
#define WIND_225			"A:/sdcard/img/w225.png"
#define WIND_270			"A:/sdcard/img/w270.png"
#define WIND_315			"A:/sdcard/img/w315.png"

#define IMG_UPDATE			"A:/sdcard/img/update.png"
#define IMG_UPDATE_OK		"A:/sdcard/img/update_ok.png"
#define IMG_UPDATE_FAIL		"A:/sdcard/img/update_fail.png"

// Пути конфигурационных файлов

#define WIFI_CONF_PATH		 "/sdcard/wifi.json"
#define SNTP_CONF_PATH		 "/sdcard/sntp.json"
#define DISPLAY_PATH		 "/sdcard/disp.json"
#define METEO_CONF_PATH		 "/sdcard/meteo.json"
#define METEO_CHART_PATH	 "/sdcard/meteo_chart.json"
#define METEO_CITY_PATH		 "/sdcard/city.json"
#define METEO_WEEK_PATH		 "/sdcard/week.json"
#define UPDATE_PATH			 "/sdcard/update.json"
#define SENSORS_PATH		 "/sdcard/sensors.json"
#define IOTV_PATH		 	 "/sdcard/iotv.json"
#define MAIN_PAGE_PATH		 "/sdcard/main_page.json"
#define TMP_FILE_TXT         "/sdcard/tmp.txt"

// Глобалные флаги статусов

enum STATUS_REG {
	STATUS_WIFI_STA_START = 0x0001,
	STATUS_WIFI_STA_CONNECTING = 0x0002,
	STATUS_WIFI_STA_CONNECTED = 0x0004,
	STATUS_IP_GOT = 0x0008,
	STATUS_WIFI_SCANNING = 0x0010,
	STATUS_WIFI_SCAN_DONE = 0x0020,
	STATUS_WIFI_AUTOCONNECT = 0x0040,
	STATUS_SNTP_ON = 0x0080,
	STATUS_METEO_ON = 0x0100,
	STATUS_METEO_CITY_SEARCH = 0x0200,
	STATUS_METEO_CITY_SEARCH_DONE = 0x0400,
	STATUS_METEO_UPDATE_NOW = 0x0800,		// Обновить данные погоды сейчас. Вызывается через GUI из меню настроек
	STATUS_DISPLAY_NIGHT_MODE_ON = 0x1000,
	STATUS_TIME_SYNC = 0x2000,
	STATUS_WIFI_AP_START = 0x4000,				// была поднята wifi станция
};

// Глобалные флаги ошибок

enum STATUS_ERROR {
	STATUS_ERROR_SD = 0x0001,
	STATUS_ERROR_UPDATE = 0x0002,
};

// Глобалные флаги обновления

enum UPDATE_FLAGS {
	UPDATE_ON = 0x01,				// Уведомлять на экране о новых обновлениях
	UPDATE_CHECK = 0x02,			// Проверить сейчас наличие обновлений
	UPDATE_AVAILABLE = 0x04,		// Доступно новое обновление
	UPDATE_DONE = 0x08,
	UPDATE_MESSAGE = 0x10,
	UPDATE_MESSAGE_NOW = 0x20,
	UPDATE_NOW = 0x40,
	UPDATE_CHECK_ERROR = 0x80,		// Ошибка файла с информацией об обновлениях
};

// Повторяющиеся строки текста в коде

#define DISPLAY_STR				 "display"
#define ROTATE_STR 				 "rotate"
#define BRIGHTNESS_STR			 "brightness"
#define NIGHT_MODE_STR			 "night_mode"
#define BRIGHTNESS_DAY_STR		 "b_day"
#define BRIGHTNESS_NIGHT_STR	 "b_night"
#define DAY_BEGIN_STR			 "d_begin"
#define NIGHT_BEGIN_STR			 "n_begin"

#define THP_STR					 "thp"
#define THP_T_CALIB_STR			 "t_calib"
#define THP_H_CALIB_STR			 "h_calib"
#define THP_P_CALIB_STR			 "p_calib"

#define IOTV_STR			 	 "iotv"
#define PORT_STR			 	 "port"

#define BLOCKS_STR				 "blocks"
#define BLOCK_0_STR				 "block_0"
#define BLOCK_1_STR				 "block_1"
#define BLOCK_2_STR				 "block_2"
#define BLOCK_3_STR				 "block_3"

#define UPDATE_STR				 "update"
#define URL_STR					 "url"
#define FIRMWARE_STR			 "firmware"
#define VERSION_STR				 "version"
#define WIFI_STR				 "wifi"
#define SSID_STR				 "ssid"
#define BSSID_STR				 "bssid"
#define PWD_STR					 "pwd"
#define AUTO_STR				 "auto"
#define SNTP_STR				 "sntp"
#define UTC_STR					 "utc"
#define CITY_STR				 "city"

#define HOURLY_STR				 "hourly"
#define TIME_STR				 "time"
#define TEMPERATURE_2M_STR		 "temperature_2m"
#define RELATIVE_HUMIDITY_2M_STR  "relative_humidity_2m"
#define APPARENT_TEMP_STR 		 "apparent_temperature"
#define PRECIPITATION_STR 		 "precipitation"
#define RAIN_STR 				 "rain"
#define SHOWERS_STR 			 "showers"
#define SNOWFALL_STR 			 "snowfall"
#define SURFACE_PRESSURE_STR 	 "surface_pressure"
#define CLOUD_COVER_STR 		 "cloud_cover"
#define WIND_SPEED_10M_STR 		 "wind_speed_10m"
#define WIND_DIRECTION_10M_STR 	 "wind_direction_10m"
#define WIND_GUSTS_10M_STR 		 "wind_gusts_10m"

#define ATTENTION_STR 			 "Внимание!"
#define ERROR_STR 				 "Ошибка!"
#define SAVE_STR 				 "Сохранить"
#define UPDATE_RU_STR			 "Обновить"
#define CANCEL_STR 				 "Отмена"
#define YES_STR 				 "Да"
#define NO_STR 					 "Нет"
#define RESET_STR				 "Сбросить"
#define HG_STR					 "мм рт.ст."


#define METEO_STR				 "meteo"
#define ON_STR					 "on"
#define LATITUDE_STR			 "latitude"
#define LONGITUDE_STR			 "longitude"

#define SETTING_STR				 "Настройки"

//fmt шаблоны

#define CANT_WRITE_FILE_TMPLT	"%s can't write \"%s\" file!\n"
#define CANT_READ_FILE_TMPLT	"%s can't read \"%s\" file!\n"
#define CANT_REMOVE_FILE_TMPLT	"%s can't remove \"%s\" file!\n"

// Константы

#define COUNTER_WEATHER			(60 * 15) 	// раз в 15 минут
#define COUNTER_CHECK_UPDATE 	(60 * 60) 	// раз в 1 час
#define COUNTER_SNTP			(60 * 1) 	// раз в 1 минуту

// Периодичность циклов служб
#define SERVICE_PERIOD_DISPLAY		1000
#define SERVICE_PERIOD_BME280		5000
#define SERVICE_PERIOD_SNTP			1000
#define SERVICE_PERIOD_TCP_CLIENT	1000
#define SERVICE_PERIOD_UPDATE		1000
#define SERVICE_PERIOD_WEATHER		1000
#define SERVICE_PERIOD_WIFI			5000
#define SERVICE_PERIOD_GUI			500


#endif /* MAIN_GLOBAL_DEF_H_ */
