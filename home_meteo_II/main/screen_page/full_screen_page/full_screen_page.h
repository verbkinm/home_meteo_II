/*
 * fuul_screen_page.h
 *
 *  Created on: 25 янв. 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_FULL_SCREEN_PAGE_FULL_SCREEN_PAGE_H_
#define MAIN_SCREEN_PAGE_FULL_SCREEN_PAGE_FULL_SCREEN_PAGE_H_

#include "lvgl.h"
#include "Global_def.h"
#include "Local_Lib/local_lvgl_lib.h"

void full_screen_page_init(const char *txt, void *img);
void full_screen_page_deinit(void);

void full_screen_page_sd_error(void);
void full_screen_page_update_https_now(void);
void full_screen_page_update_sd_now(void);
void full_screen_page_update_done(void);
void full_screen_page_update_fail(void);

#endif /* MAIN_SCREEN_PAGE_FULL_SCREEN_PAGE_FULL_SCREEN_PAGE_H_ */
