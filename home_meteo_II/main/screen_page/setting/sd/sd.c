/*
 * sd.c
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#include "sd.h"

#include "Local_Lib/local_lib.h"
#include "screen_page/setting/settingpage.h"
#include "screen_page/setting/elements.h"
#include "sd/sd_spi.h"

extern lv_obj_t *sub_sd_page;

//static const char *TAG = "SETTING SD";

void create_sd_sub_page(lv_event_t *e)
{
	clear_all_sub_page_child();

	lv_obj_set_style_pad_hor(sub_sd_page, 20, 0);
	lv_obj_t *section = lv_menu_section_create(sub_sd_page);

	char *buf = calloc(1, BUFSIZE);
	if (buf != NULL)
	{
		if (sd_spi_info(buf, BUFSIZE - 1) == ESP_OK)
			create_text(section, NULL, buf, LV_MENU_ITEM_BUILDER_VAR_1);

		free(buf);
	}
}

void free_sd_sub_page(void)
{

}
