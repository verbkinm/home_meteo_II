/*
 * Аналгоговые часы
 *
 * datetime3.h
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#ifndef MAIN_SCREEN_PAGE_DATETIME3_DATETIME3_H_
#define MAIN_SCREEN_PAGE_DATETIME3_DATETIME3_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "Global_def.h"
#include "Local_Lib/local_lvgl_lib.h"
#include "screen_page/screendefault.h"
#include "screen_page/datetime1/datetime1.h"
#include "screen_page/page.h"
#include "status_panel/status_panel.h"

void datetime3_page_init(void);
void datetime3_page_deinit(void);

#endif /* MAIN_SCREEN_PAGE_DATETIME3_DATETIME3_H_ */
