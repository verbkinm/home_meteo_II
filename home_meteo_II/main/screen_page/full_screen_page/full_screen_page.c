/*
 * full_screen_page.c
 *
 *  Created on: 25 янв. 2024 г.
 *      Author: user
 */

#include "full_screen_page.h"

extern lv_font_t ubuntu_mono_48;

void full_screen_page_init(const char *txt, void *img)
{
	page_t *page = page_current();
	page->deinit();
	page->deinit = full_screen_page_deinit;
	page->title = page_title(PAGE_TITLE_EMPTY);
	lv_obj_clear_flag(page_current()->fullscreen, LV_OBJ_FLAG_HIDDEN);
//	page->num = page_num;

	lv_obj_clean(lv_layer_top());
	lv_obj_t *fullscreen = page->fullscreen;//lv_obj_create(lv_scr_act());
//	page->widget = main_widget;

	lv_obj_set_size(fullscreen, LCD_H_RES, LCD_V_RES);
	lv_obj_set_style_radius(fullscreen, 0, 0);
	lv_obj_t *img_sd = lv_img_create(fullscreen);
	lv_obj_set_size(img_sd, 256, 256);
	lv_obj_center(img_sd);
	lv_img_set_src(img_sd, img);

	lv_obj_t *lbl = lv_label_create(fullscreen);
	lv_label_set_text(lbl, txt);
	lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
	lv_obj_set_style_text_font(lbl, &ubuntu_mono_48, 0);
}

void full_screen_page_sd_error(void)
{
	LV_IMG_DECLARE(sd);
	full_screen_page_init("Ошибка SD карты!", (void *)&sd);
	page_current()->num = PAGE_SYS_SD_ERROR;
}

void full_screen_page_update_https_now(void)
{
	full_screen_page_init("Обновление (HTTPS) ...", IMG_UPDATE);
	page_current()->num = PAGE_SYS_UPDATE_NOW;
}

void full_screen_page_update_sd_now(void)
{
	full_screen_page_init("Обновление (SD) ...", IMG_UPDATE);
	page_current()->num = PAGE_SYS_UPDATE_NOW;
}

void full_screen_page_update_done(void)
{
	full_screen_page_init("Перезагрузите устройство...", IMG_UPDATE_OK);
	page_current()->num = PAGE_SYS_UPDATE_DONE;
}

void full_screen_page_update_fail(void)
{
	full_screen_page_init("Ошибка обновления!", IMG_UPDATE_FAIL);
	page_current()->num = PAGE_SYS_UPDATE_ERROR;
}

void full_screen_page_deinit(void)
{
	page_default_deinit();
}
