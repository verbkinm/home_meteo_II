/*
 * wifi.c
 *
 *  Created on: 14 дек. 2023 г.
 *      Author: user
 */

#include "wifi.h"

static const char *TAG = "WIFI";

esp_netif_t *sta_netif;

static void wifi_init(void);
static void read_wifi_conf(void);

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void check_wifi_conf_file(void);

static void wifi_init(void)
{
	//	// Initialize NVS
	//	esp_err_t ret = nvs_flash_init();
	//	ESP_ERROR_CHECK(nvs_flash_erase());
	//	ret = nvs_flash_init();
	//	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	//		ESP_ERROR_CHECK(nvs_flash_erase());
	//		ret = nvs_flash_init();
	//	}
	//	ESP_ERROR_CHECK(ret);
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	sta_netif = esp_netif_create_default_wifi_sta();


	//
	esp_netif_create_default_wifi_ap();


	const char* hostname = HOSTNAME;
	ESP_ERROR_CHECK(esp_netif_set_hostname(sta_netif, hostname));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			&event_handler,
			NULL,
			&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
			ESP_EVENT_ANY_ID,
			&event_handler,
			NULL,
			&instance_any_id));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

	wifi_config_t wifi_config;
	esp_wifi_get_config(WIFI_IF_AP, &wifi_config);

	char *ssid = "AP";
	char *password = "01234567";

	memcpy(wifi_config.ap.ssid, ssid, 2);
	memcpy(wifi_config.ap.password, password, 8);
	wifi_config.ap.ssid_len = 2;
	wifi_config.ap.ssid_hidden = 1;
	wifi_config.ap.max_connection = 1;
	wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
	wifi_config.ap.pmf_cfg.required = true;

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

	check_wifi_conf_file();
	read_wifi_conf();
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		printf("%s sta start\n", WIFI_STR);
		glob_set_bits_status_reg(STATUS_WIFI_STA_START);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTING);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTED);
		glob_clear_bits_status_reg(STATUS_WIFI_SCANNING);
		glob_clear_bits_status_reg(STATUS_WIFI_SCAN_DONE);
		glob_clear_bits_status_reg(STATUS_IP_GOT);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP)
	{
		printf("%s sta stop\n", WIFI_STR);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_START);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTING);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTED);
		glob_clear_bits_status_reg(STATUS_WIFI_SCANNING);
		glob_clear_bits_status_reg(STATUS_WIFI_SCAN_DONE);
		glob_clear_bits_status_reg(STATUS_IP_GOT);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
	{
		printf("%s sta connected\n", WIFI_STR);
		glob_set_bits_status_reg(STATUS_WIFI_STA_CONNECTED);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTING);

		wifi_config_t wifi_config;
		esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

		char mac_str[18] = {0};
		char pwd_str[65] = {0};
		char ssid_str[32] = {0};

		sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
				wifi_config.sta.bssid[0], wifi_config.sta.bssid[1], wifi_config.sta.bssid[2],
				wifi_config.sta.bssid[3], wifi_config.sta.bssid[4], wifi_config.sta.bssid[5]);
		sprintf(pwd_str, "%s", wifi_config.sta.password);
		sprintf(ssid_str, "%s", wifi_config.sta.ssid);

		wifi_ap_config_t *ap_info = (wifi_ap_config_t *)event_data;
		printf("ap_info->ssid %s\n", ap_info->ssid);

		set_wifi_config_value(SSID_STR, ssid_str);
		set_wifi_config_value(BSSID_STR, mac_str);
		bool res = set_wifi_config_value(PWD_STR, pwd_str);
		set_wifi_config_value(AUTO_STR, "1");
		glob_set_bits_status_reg(STATUS_WIFI_AUTOCONNECT);

		printf("write pwd %d, pwd = %s\n", (int)res, pwd_str);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		printf("%s sta disconnect\n", WIFI_STR);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTED);
		glob_clear_bits_status_reg(STATUS_WIFI_STA_CONNECTING);
		//		glob_status_reg &= ~STATUS_WIFI_SCANNING;
		//		glob_status_reg &= ~STATUS_WIFI_SCAN_DONE;
		glob_clear_bits_status_reg(STATUS_IP_GOT);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
	{
		glob_clear_bits_status_reg(STATUS_WIFI_SCANNING);
		glob_set_bits_status_reg(STATUS_WIFI_SCAN_DONE);
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		glob_set_bits_status_reg(STATUS_IP_GOT);
		printf("%s sta got ip\n", WIFI_STR);
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
	{
		printf("%s sta ip lost\n", WIFI_STR);
		glob_clear_bits_status_reg(STATUS_IP_GOT);
	}
	else if (event_base == WIFI_EVENT)
		printf("%s other event: %s\n", WIFI_STR, event_base);

	if (event_id == WIFI_EVENT_AP_START)
	{
		ESP_LOGI(TAG, "AP up");
		glob_set_bits_status_reg(STATUS_WIFI_AP_START);
	}
	else if (event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
		ESP_LOGI(TAG, "station join, AID=%d", event->aid);
	}
	else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
		ESP_LOGI(TAG, "station leave, AID=%d", event->aid);
	}
}

static void check_wifi_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();

	cJSON *wifi = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(root, WIFI_STR, wifi);

	cJSON *on_obj = cJSON_CreateString("0");
	cJSON_AddItemToObject(wifi, ON_STR, on_obj);

	cJSON *auto_con_obj = cJSON_CreateString("0");
	cJSON_AddItemToObjectCS(wifi, AUTO_STR, auto_con_obj);

	cJSON *ssid_obj = cJSON_CreateString("");
	cJSON_AddItemToObjectCS(wifi, SSID_STR, ssid_obj);

	cJSON *pwd_obj = cJSON_CreateString("");
	cJSON_AddItemToObjectCS(wifi, PWD_STR, pwd_obj);

	get_wifi_config_value(ON_STR, &on_obj->valuestring);
	get_wifi_config_value(AUTO_STR, &auto_con_obj->valuestring);
	get_wifi_config_value(SSID_STR, &ssid_obj->valuestring);
	get_wifi_config_value(PWD_STR, &pwd_obj->valuestring);

	FILE *file = fopen(WIFI_CONF_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, WIFI_CONF_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	cJSON_Delete(root);
}

static void read_wifi_conf(void)
{
	char *on = NULL;
	char *auto_on = NULL;
	char *ssid = NULL;
	char *pwd = NULL;

	wifi_config_t wifi_config;
	esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

	// Автоматическое включение wifi
	if (get_wifi_config_value(ON_STR, &on) && on != NULL)
	{
		if (strcmp("1", on) == 0)
			esp_wifi_start();

		//		printf("read wifi conf. on = %s\n", on);
		free(on);
	}

	// SSID точки доступа
	if (get_wifi_config_value(SSID_STR, &ssid) && ssid != NULL)
	{
		strcpy((char *)wifi_config.sta.ssid, ssid);
		//		printf("read wifi conf. ssid = %s\n", ssid);
		free(ssid);
	}

	// Пароль точки доступа
	if (get_wifi_config_value(PWD_STR, &pwd) && pwd != NULL)
	{
		strcpy((char *)wifi_config.sta.password, pwd);
		//		printf("read wifi conf. pwd = %s\n", pwd);
		free(pwd);
	}

	// Автоматическое подключение
	if (get_wifi_config_value(AUTO_STR, &auto_on) && auto_on != NULL)
	{
		if (strcmp("1", auto_on) == 0)
			glob_set_bits_status_reg(STATUS_WIFI_AUTOCONNECT);
		else
			glob_clear_bits_status_reg(STATUS_WIFI_AUTOCONNECT);
		//		printf("read wifi conf. auto = %s\n", auto_on);
		free(auto_on);
	}

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
}

void service_wifi_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);
	wifi_init();

	for( ;; )
	{
		//		printf("WIFI from core %d!\n", xPortGetCoreID() );

		if (glob_get_status_err())
			break;

		if ( !(glob_get_status_reg() & STATUS_WIFI_AUTOCONNECT)
				|| !(glob_get_status_reg() & STATUS_WIFI_STA_START)
				|| (glob_get_status_reg() & STATUS_WIFI_STA_CONNECTING) )
			goto for_end;

		if ( !(glob_get_status_reg() & STATUS_WIFI_STA_CONNECTED) )
		{
			wifi_config_t wifi_config;
			esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

			glob_set_bits_status_reg(STATUS_WIFI_STA_CONNECTING);
			esp_wifi_connect();
		}

		for_end:
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
