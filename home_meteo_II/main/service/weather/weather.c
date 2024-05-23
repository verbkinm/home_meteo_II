/*
 * wearther.c
 *
 *  Created on: 18 дек. 2023 г.
 *      Author: user
 */

#include "weather.h"

static char *city_search = NULL;

static const char *TAG = "WEATHER";

static open_meteo_data_t *open_meteo = NULL;
static const char *city_url = "https://geocoding-api.open-meteo.com/v1/search?count=20&language=ru&format=json&name=";
static const char *meteo_url = "https://api.open-meteo.com/v1/forecast?forecast_days=3&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,rain,showers,snowfall,surface_pressure,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m&timeformat=unixtime&timezone=auto&latitude=";

static uint16_t counter = COUNTER_WEATHER;

static void check_meteo_conf_file(void);
static char *generate_url_meteo(void);
static bool parse_open_meteo_weather(const char *data, open_meteo_data_t *open_meteo_week);
static void http_city_search(void);
static void http_meteo_to_file(void);
static bool service_weather_parse_meteo_data(void);
static esp_err_t _http_event_handler(esp_http_client_event_t *evt);

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	case HTTP_EVENT_REDIRECT:
		ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
		break;
	}
	return ESP_OK;
}

static void check_meteo_conf_file(void)
{
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
		return;

	cJSON *meteo = cJSON_CreateObject();

	if (meteo == NULL)
		goto end;

	cJSON_AddItemToObjectCS(root, METEO_STR, meteo);

	cJSON *on_obj = cJSON_CreateString("0");
	if (on_obj == NULL)
		goto end;

	cJSON_AddItemToObject(meteo, ON_STR, on_obj);

	cJSON *city_obj = cJSON_CreateString("Москва");
	if (city_obj == NULL)
		goto end;

	cJSON_AddItemToObject(meteo, CITY_STR, city_obj);

	cJSON *latitude_obj = cJSON_CreateString("55.75222");
	if (latitude_obj == NULL)
		goto end;

	cJSON_AddItemToObject(meteo, LATITUDE_STR, latitude_obj);

	cJSON *longitude_obj = cJSON_CreateString("37.61556");
	if (longitude_obj == NULL)
		goto end;

	cJSON_AddItemToObject(meteo, LONGITUDE_STR, longitude_obj);

	get_meteo_config_value(ON_STR, &on_obj->valuestring);
	get_meteo_config_value(CITY_STR, &city_obj->valuestring);
	get_meteo_config_value(LATITUDE_STR, &latitude_obj->valuestring);
	get_meteo_config_value(LONGITUDE_STR, &longitude_obj->valuestring);

	FILE *file = fopen(METEO_CONF_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, METEO_WEEK_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	end:
	cJSON_Delete(root);
}

static void http_city_search(void)
{
	ESP_LOGI(TAG, "http_city_search");

	char *response_buffer = calloc(1, BUFSIZ);
	if (response_buffer == NULL)
		return;

	char *city = url_encode(city_search);
	if (city == NULL)
	{
		free(response_buffer);
		return;
	}

	int len = strlen(city_url) + strlen(city);

	char *url = calloc(1, len + 1);
	if (url == NULL)
	{
		free(response_buffer);
		free(city);
		return;
	}

	strcat(url, city_url);
	strcat(url, city);
	free(city);

	esp_http_client_config_t config = {
			.url = url,
			.crt_bundle_attach = esp_crt_bundle_attach,
			.event_handler = _http_event_handler,
	};

	esp_http_client_handle_t client = esp_http_client_init(&config);
	if (esp_http_client_open(client, 0) != ESP_OK)
		goto end;

	esp_http_client_fetch_headers(client);

	FILE *file = fopen(METEO_CITY_PATH, "w");
	if (file == NULL)
	{
		printf(CANT_WRITE_FILE_TMPLT, TAG, METEO_CITY_PATH);
		goto end;
	}

	int counter = 0;
	while (true)
	{
		int ret = esp_http_client_read(client, response_buffer, BUFSIZ);
		if (ret < 1)
			break;

		fwrite(response_buffer, ret, 1, file);
		if (counter++ > 250)
			break;
	}
	fclose(file);

	end:
	free(url);
	free(response_buffer);
	esp_http_client_cleanup(client);

	glob_set_bits_status_reg(STATUS_METEO_CITY_SEARCH_DONE);
}

static char *generate_url_meteo(void)
{
	char *url = NULL;
	const char *second_part = "&longitude=";

	char *latitude = NULL;
	get_meteo_config_value(LATITUDE_STR, &latitude);
	if (latitude == NULL)
	{
		ESP_LOGE(TAG, "%s error", LATITUDE_STR);
		return NULL;
	}

	char *longitude = NULL;
	get_meteo_config_value(LONGITUDE_STR, &longitude);
	if (longitude == NULL)
	{
		ESP_LOGE(TAG, "%s error", LONGITUDE_STR);
		free(latitude);
		return NULL;
	}

	int len = strlen(meteo_url) + strlen(latitude) + strlen(second_part) + strlen(longitude);

	url = calloc(1, len + 1);
	if (url == NULL)
		goto end;

	strcat(url, meteo_url);
	strcat(url, latitude);
	strcat(url, second_part);
	strcat(url, longitude);

	end:
	free(latitude);
	free(longitude);

	return url;
}

void service_weather_read_conf(void)
{
	char *on = NULL;
	if (get_meteo_config_value(ON_STR, &on) && on != NULL)
	{
		if (strcmp(on, "1") == 0)
			glob_set_bits_status_reg(STATUS_METEO_ON);

		free(on);
	}

	char *city = NULL;
	if (get_meteo_config_value(CITY_STR, &city) && city != NULL)
	{
		service_weather_set_city(city);
		free(city);
	}
}

static bool parse_open_meteo_weather(const char *data, open_meteo_data_t *open_meteo_week)
{
	if (data == NULL || open_meteo_week == NULL)
		return false;

	cJSON *monitor = cJSON_Parse(data);
	if (monitor == NULL)
		return false;

	cJSON *obj_parent = cJSON_GetObjectItemCaseSensitive(monitor, HOURLY_STR);
	if (obj_parent == NULL)
		goto bad_end;

	cJSON *obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, TIME_STR);
	if (obj_child == NULL)
		goto bad_end;

	cJSON *obj;
	int i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].time = cJSON_GetNumberValue(obj);
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, TEMPERATURE_2M_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].temperature = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child = cJSON_GetObjectItemCaseSensitive(obj_parent, RELATIVE_HUMIDITY_2M_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].relative_humidity = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, APPARENT_TEMP_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].apparent_temperature = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, PRECIPITATION_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].precipitation = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, RAIN_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].rain = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, SHOWERS_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].showers = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, SNOWFALL_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].snowfall = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, SURFACE_PRESSURE_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].surface_pressure = cJSON_GetNumberValue(obj) * 0.75006; // hPa to mm Hg
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, CLOUD_COVER_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].cloud_cover = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, WIND_SPEED_10M_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].wind_speed = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, WIND_DIRECTION_10M_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].wind_direction = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	obj_child= cJSON_GetObjectItemCaseSensitive(obj_parent, WIND_GUSTS_10M_STR);
	if (obj_child == NULL)
		goto bad_end;

	i = 0;
	cJSON_ArrayForEach(obj, obj_child)
	{
		open_meteo_week[i++].wind_gusts = cJSON_GetNumberValue(obj);
		if (i > OPEN_METEO_WEEK_SIZE)
			goto bad_end;
	}

	cJSON_Delete(monitor);
	return true;

	bad_end:
	cJSON_Delete(monitor);
	return false;
}

const open_meteo_data_t *service_weather_get_current_meteo_data(void)
{
	if (open_meteo == NULL)
		return NULL;

	time_t now;
	time(&now);

	if (now < open_meteo[0].time)
		return &open_meteo[0];

	for (int i = 1; i < OPEN_METEO_WEEK_SIZE - 1; ++i)
	{
		if (now > open_meteo[i - 1].time && now < open_meteo[i + 1].time)
		{
			if (now > (open_meteo[i + 1].time - (60 * 30)))
				return &open_meteo[i + 1];
			else if (now < (open_meteo[i - 1].time + (60 * 30)))
				return &open_meteo[i - 1];
			else
				return &open_meteo[i];
		}
	}

	if (now >= open_meteo[OPEN_METEO_WEEK_SIZE - 1].time)
		return &open_meteo[OPEN_METEO_WEEK_SIZE - 1];

	return NULL;
}

const open_meteo_data_t *service_weather_get_meteo_data(void)
{
	return open_meteo;
}

void service_weather_get_range(open_meteo_data_t *ret_min, open_meteo_data_t *ret_max)
{
	if (open_meteo == NULL || ret_min == NULL || ret_max == NULL)
		return;

	memcpy(ret_min, &open_meteo[0], sizeof(struct Open_Meteo_Data));
	memcpy(ret_max, &open_meteo[0], sizeof(struct Open_Meteo_Data));

	for (int i = 1; i < OPEN_METEO_WEEK_SIZE; ++i)
	{
		ret_max->temperature = MAX(ret_max->temperature, open_meteo[i].temperature);
		ret_max->apparent_temperature = MAX(ret_max->apparent_temperature, open_meteo[i].apparent_temperature);
		ret_max->surface_pressure = MAX(ret_max->surface_pressure, open_meteo[i].surface_pressure);
		ret_max->precipitation = MAX(ret_max->precipitation, open_meteo[i].precipitation);
		ret_max->rain = MAX(ret_max->rain, open_meteo[i].rain);
		ret_max->showers = MAX(ret_max->showers, open_meteo[i].showers);
		ret_max->snowfall = MAX(ret_max->snowfall, open_meteo[i].snowfall);
		ret_max->wind_speed = MAX(ret_max->wind_speed, open_meteo[i].wind_speed);
		ret_max->wind_gusts = MAX(ret_max->wind_gusts, open_meteo[i].wind_gusts);
		ret_max->relative_humidity = MAX(ret_max->relative_humidity, open_meteo[i].relative_humidity);
		ret_max->cloud_cover = MAX(ret_max->cloud_cover, open_meteo[i].cloud_cover);
		ret_max->wind_direction = MAX(ret_max->wind_direction, open_meteo[i].wind_direction);

		ret_min->temperature = MIN(ret_min->temperature, open_meteo[i].temperature);
		ret_min->apparent_temperature = MIN(ret_min->apparent_temperature, open_meteo[i].apparent_temperature);
		ret_min->surface_pressure = MIN(ret_min->surface_pressure, open_meteo[i].surface_pressure);
		ret_min->precipitation = MIN(ret_min->precipitation, open_meteo[i].precipitation);
		ret_min->rain = MIN(ret_min->rain, open_meteo[i].rain);
		ret_min->showers = MIN(ret_min->showers, open_meteo[i].showers);
		ret_min->snowfall = MIN(ret_min->snowfall, open_meteo[i].snowfall);
		ret_min->wind_speed = MIN(ret_min->wind_speed, open_meteo[i].wind_speed);
		ret_min->wind_gusts = MIN(ret_min->wind_gusts, open_meteo[i].wind_gusts);
		ret_min->relative_humidity = MIN(ret_min->relative_humidity, open_meteo[i].relative_humidity);
		ret_min->cloud_cover = MIN(ret_min->cloud_cover, open_meteo[i].cloud_cover);
		ret_min->wind_direction = MIN(ret_min->wind_direction, open_meteo[i].wind_direction);
	}
}

void service_weather_set_city(const char* city)
{
	if (city == NULL)
		return;

	if (city_search != NULL)
		free(city_search);

	city_search = calloc(1, strlen(city) + 1);
	if (city_search != NULL)
		strcpy(city_search, city);
}

const char *service_weather_get_city(void)
{
	return city_search;
}

static void http_meteo_to_file(void)
{
	char *response_buffer = malloc(BUFSIZE);
	if (response_buffer == NULL)
	{
		ESP_LOGE(TAG, "response_buffer = NULL");
		return;
	}

	char *url = generate_url_meteo();
	if (url == NULL)
	{
		ESP_LOGE(TAG, "url = NULL");
		free(response_buffer);
		return;
	}

	esp_http_client_config_t config = {
			.url = url,
			.crt_bundle_attach = esp_crt_bundle_attach,
			.event_handler = _http_event_handler,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	if (esp_http_client_open(client, 0) != ESP_OK)
		goto end;

	esp_http_client_fetch_headers(client);

	FILE *file = fopen(METEO_WEEK_PATH, "w");
	if (file == NULL)
	{
		printf(CANT_WRITE_FILE_TMPLT, TAG, METEO_WEEK_PATH);
		goto end;
	}

	int counter = 0;
	while (true)
	{
		int ret = esp_http_client_read(client, response_buffer, BUFSIZE);
		if (ret < 1)
			break;

		fwrite(response_buffer, ret, 1, file);
//		vTaskDelay(10 / portTICK_PERIOD_MS);

		if (counter++ > 250) // 250 - ???
			break;
	}

	fclose(file);

	end:
	esp_http_client_cleanup(client);
	free(url);
	free(response_buffer);
}

static bool service_weather_parse_meteo_data(void)
{
	bool result = false;

	struct stat st;
	stat(METEO_WEEK_PATH, &st);
	size_t size = st.st_size;

	if (size == 0)
	{
		ESP_LOGE(TAG, "size = 0");
		return false;
	}

	char *content = calloc(1, size + 1);
	if (content == NULL)
	{
		ESP_LOGE(TAG, "content error calloc");
		return false;
	}

	FILE *file = fopen(METEO_WEEK_PATH, "r");
	if (file == NULL)
	{
		printf(CANT_READ_FILE_TMPLT, TAG, METEO_WEEK_PATH);
		goto end;
	}

	fread(content, size, 1, file);
	fclose(file);

	if (open_meteo != NULL)
		free(open_meteo);

	open_meteo = calloc(OPEN_METEO_WEEK_SIZE, sizeof(open_meteo_data_t));
	if (open_meteo == NULL)
	{
		ESP_LOGE(TAG, "open_meteo = NULL");
		goto end;
	}

	if (!parse_open_meteo_weather(content, open_meteo))
	{
		ESP_LOGE(TAG, "error parse_open_meteo_weather");
		free(open_meteo);
		open_meteo = NULL;
		goto end;
	}

	result = true;

	end:
	free(content);

	return result;
}

//static void debug_open_meteo()
//{
//	if (open_meteo == NULL)
//		return;
//
//	for (int i = 0; i < OPEN_METEO_WEEK_SIZE; ++i)
//	{
//		printf("%d:\ntime: %lld\n", i, open_meteo[i].time);
//	}
//}

void service_weather_update(void)
{
	counter = COUNTER_WEATHER;
}

void service_weather_task(void *pvParameters)
{
	vTaskDelay(DELAYED_LAUNCH / portTICK_PERIOD_MS);

	check_meteo_conf_file();
	service_weather_read_conf();


	for( ;; )
	{
		if (glob_get_status_err()
				|| (glob_get_update_reg() & UPDATE_NOW))
			break;

		if ( !(glob_get_status_reg() & STATUS_IP_GOT)
				|| !(glob_get_status_reg() & STATUS_METEO_ON))
			goto for_end;

		if (glob_get_status_reg() & STATUS_METEO_CITY_SEARCH)
			http_city_search();

		if (glob_get_status_reg() & STATUS_METEO_UPDATE_NOW)
		{
			glob_clear_bits_status_reg(STATUS_METEO_UPDATE_NOW);
			counter = COUNTER_WEATHER;
		}

		if (++counter < COUNTER_WEATHER)
			goto for_end;

		counter = 0;

		http_meteo_to_file();
		if (service_weather_parse_meteo_data() == false)
			counter = COUNTER_WEATHER;

		for_end:
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
