#include "TFT_touchscreen/TFT_touch_screen.h"
#include "I2C/i2c.h"
#include "sd/sd_spi.h"
#include "iotv.h"

#include "GUI_manager/GUI_manager.h"

#include "nvs_flash.h"

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
//	ESP_ERROR_CHECK(nvs_flash_erase());
//	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	iotv_init();

	sd_spi_init();

	i2c_init();
	DS3231_set_system_time();

	TFT_init(); // Перед этой функцией обязательно выполнить i2c_init();

	start_services();

	GUI_manager_init();

	while (1)
	{
		lv_timer_handler();
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
}

//ИСПРАВЛЕНО FIXME: При ошибках считывания BME280 графический интерфейс в настройках "Датчики" сильно тормозит!
//FIXME: О продукте
//FIXME: Везде, где проверяется strlen необходимо с начала проверить, что указатель на строку не равен NULL
//FIXME: Названия показателей погоды объеденить между homePage и meteoChartPage. Использовать replace_char для замены пробелов на символ переноса строки

/* Online Font Converter https://lvgl.io/tools/fontconverter
а бвгдеёжзийклмнопрстуфхцчшщъыьэюя
АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
0123456789
.,°
`~!@#$%^&*()_+-=
!"№;:?<>/\|

 */
