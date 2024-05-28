/*
 * homepage.c
 *
 *  Created on: 1 дек. 2023 г.
 *      Author: user
 */

#include "homepage.h"

#include "Global_def.h"
#include "service/bme280/bme280.h"
#include "screen_page/screendefault.h"
#include "service/weather/weather.h"
#include "status_panel/status_panel.h"
#include "block/block.h"

//#define BLOCK1_COUNTER_SEC		5
#define MAIN_PAGE_BLOCK_SIZE	4
static const char *TAG = "homepage";

static lv_obj_t *blocks[MAIN_PAGE_BLOCK_SIZE];

static lv_timer_t *timer = NULL;

static void timer_handler(lv_timer_t *timer);
static void homepage_check_conf_file(void);

static void homepage_check_conf_file(void)
{
	char buf[8] = {0};

	cJSON *root = cJSON_CreateObject();

	cJSON *block = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(root, BLOCKS_STR, block);

	snprintf(buf, sizeof(buf) - 1, "%hhu", BLOCKTYPE_LOCALSENSOR_1_4);
	cJSON *block_0 = cJSON_CreateString(buf);
	cJSON_AddItemToObject(block, BLOCK_0_STR, block_0);

	snprintf(buf, sizeof(buf) - 1, "%hhu", BLOCKTYPE_WEATHER_1_4);
	cJSON *block_1 = cJSON_CreateString(buf);
	cJSON_AddItemToObject(block, BLOCK_1_STR, block_1);

	snprintf(buf, sizeof(buf) - 1, "%hhu", BLOCKTYPE_DATETIME_1_4);
	cJSON *block_2 = cJSON_CreateString(buf);
	cJSON_AddItemToObject(block, BLOCK_2_STR, block_2);

	snprintf(buf, sizeof(buf) - 1, "%hhu", BLOCKTYPE_REMOTESENSOR_1_4);
	cJSON *block_3 = cJSON_CreateString(buf);
	cJSON_AddItemToObject(block, BLOCK_3_STR, block_3);

	get_main_page_config_value(BLOCK_0_STR, &block_0->valuestring);
	get_main_page_config_value(BLOCK_1_STR, &block_1->valuestring);
	get_main_page_config_value(BLOCK_2_STR, &block_2->valuestring);
	get_main_page_config_value(BLOCK_3_STR, &block_3->valuestring);

	FILE *file = fopen(MAIN_PAGE_PATH, "w");
	if (file == NULL)
		printf(CANT_WRITE_FILE_TMPLT, TAG, MAIN_PAGE_PATH);
	else
	{
		fprintf(file, "%s", cJSON_Print(root));
		fclose(file);
	}

	cJSON_Delete(root);
}

static void timer_handler(lv_timer_t *timer)
{
	if (glob_get_status_err())
	{
		lv_timer_pause(timer);
		return;
	}

	for (int i = 0; i < MAIN_PAGE_BLOCK_SIZE; ++i)
		block_update(blocks[i]);
}

void homePageInit()
{
	page_t *page = page_current();
	page->deinit();
	page->deinit = home_page_deinit;
	page->title = page_title(PAGE_TITLE_MAIN);
	page->num = PAGE_MAIN;

	status_panel_update();

	homepage_check_conf_file();

	const char *blocks_str[] = {BLOCK_0_STR, BLOCK_1_STR, BLOCK_2_STR, BLOCK_3_STR};
	const lv_align_t blocks_align[] = {LV_ALIGN_TOP_LEFT, LV_ALIGN_TOP_RIGHT, LV_ALIGN_BOTTOM_LEFT, LV_ALIGN_BOTTOM_RIGHT};

	for (int i = 0; i < MAIN_PAGE_BLOCK_SIZE; ++i)
	{
		char *obj = NULL;
		if (get_main_page_config_value(blocks_str[i], &obj) && obj != NULL)
		{
			block_type_t bt = strtol(obj, NULL, 10);
			bt = inRange(bt, BLOCKTYPE_NONE, BLOCKTYPE_NUM - 1);
			blocks[i] = block_create(page->widget, bt);
			if (blocks[i] != NULL)
				lv_obj_align(blocks[i], blocks_align[i], 0, 0);

			free(obj);
		}
	}

	timer = lv_timer_create(timer_handler, 1000, NULL);
	lv_timer_ready(timer);
}

void home_page_deinit(void)
{
	page_default_deinit();
	lv_timer_del(timer);
}
