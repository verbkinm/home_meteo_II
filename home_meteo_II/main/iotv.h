#ifndef MAIN_IOTV_H_
#define MAIN_IOTV_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdint.h>

#include "Protocols/embedded/iotv_types.h"
#include "Protocols/embedded/creatorpkgs.h"
#include "Protocols/embedded/IOTV_SH.h"
#include "Protocols/embedded/iotv_server_embedded.h"
#include "Global_def.h"

void service_iotv_task(void *pvParameters);
void iotv_data_recived(const char *data, int size, int sock);

void iotv_clear_buf_data(void);

const struct IOTV_Server_embedded *iotv_get(void);

#endif /* MAIN_IOTV_H_ */
