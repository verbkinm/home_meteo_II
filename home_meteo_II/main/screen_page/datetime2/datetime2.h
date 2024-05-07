/*
 * Аналгоговые часы
 *
 * datetime2.h
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_DATETIME2_DATETIME2_H_
#define MAIN_SCREEN_PAGE_DATETIME2_DATETIME2_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "Global_def.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "screen_page/screendefault.h"
#include "screen_page/page.h"
#include "screen_page/datetime3/datetime3.h"
#include "status_panel/status_panel.h"

void datetime2_page_init(void);
void datetime2_page_deinit(void);

#endif /* MAIN_SCREEN_PAGE_DATETIME2_DATETIME2_H_ */
