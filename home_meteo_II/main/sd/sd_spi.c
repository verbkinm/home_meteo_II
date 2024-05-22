/*
 * sd_spi.c
 *
 *  Created on: 9 дек. 2023 г.
 *      Author: user
 */

#include "sd_spi.h"

#define MOUNT_POINT "/sdcard"

static char *TAG = "SD SPI";
static sdmmc_card_t *card;

static esp_err_t wrap_sdspi_host_do_transaction(int slot, sdmmc_command_t *cmdinfo);

static esp_err_t wrap_sdspi_host_do_transaction(int slot, sdmmc_command_t *cmdinfo)
{
	esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);

	if (ret == ESP_ERR_TIMEOUT)
	{
		static time_t old_t = 0;
		if (old_t == 0)
		{
			time(&old_t);
			return ret;
		}

		time_t cur_t;
		time(&cur_t);

		if ((cur_t - old_t) == 0)
		{
			glob_set_bits_status_err(STATUS_ERROR_SD);
			ESP_LOGE(TAG, "STATUS_SD_ERROR - %d", ret);
			return ret;
		}
		time(&old_t);
	}

	if (ret != ESP_OK && ret != ESP_ERR_NOT_SUPPORTED)
	{
		ESP_LOGE(TAG, "STATUS_SD_ERROR - %d", ret);
		glob_set_bits_status_err(STATUS_ERROR_SD);
	}

	return ret;
}


int sd_spi_info(char *buf, size_t buf_size)
{
	if (buf == NULL)
		return ESP_FAIL;

	sdmmc_card_t *card = sd_spi_card();

	FILE *stream = fopen(TMP_FILE_TXT, "w+");
	if (stream == NULL)
	{
		printf(CANT_WRITE_FILE_TMPLT, TAG, TMP_FILE_TXT);
		return ESP_FAIL;
	}

	// Переделанный исходник из функции sdmmc_card_print_info файла библиотеки "sdmmc_common.с"
	bool print_scr = false;
	bool print_csd = false;
	const char* type;

	fprintf(stream, "Имя: %s\n", card->cid.name);

	if (card->is_sdio)
	{
		type = "SDIO";
		print_scr = true;
		print_csd = true;
	}
	else if (card->is_mmc)
	{
		type = "MMC";
		print_csd = true;
	}
	else
	{
#define SD_OCR_SDHC_CAP (1<<30)
		type = (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
		print_csd = true;
	}

	fprintf(stream, "Тип: %s\n", type);

	if (card->real_freq_khz == 0)
		fprintf(stream, "Скорость: N/A\n");
	else
	{
		const char *freq_unit = card->real_freq_khz < 1000 ? "kHz" : "MHz";
		const float freq = card->real_freq_khz < 1000 ? card->real_freq_khz : card->real_freq_khz / 1000.0;
		const char *max_freq_unit = card->max_freq_khz < 1000 ? "kHz" : "MHz";
		const float max_freq = card->max_freq_khz < 1000 ? card->max_freq_khz : card->max_freq_khz / 1000.0;
		fprintf(stream, "Скорость: %.2f %s (limit: %.2f %s)%s\n", freq, freq_unit, max_freq, max_freq_unit, card->is_ddr ? ", DDR" : "");
	}

	uint32_t totalKB = 0, availableKB = 0;
	sd_spi_space_info(&totalKB, &availableKB);

	fprintf(stream, "Размер: %lu МБ, доступно: %lu МБ\n", totalKB / 1024, availableKB / 1024);

	if (print_csd)
	{
		fprintf(stream, "CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
				(card->is_mmc ? card->csd.csd_ver : card->csd.csd_ver + 1),
				card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
		if (card->is_mmc)
			fprintf(stream, "EXT CSD: bus_width=%d\n", (1 << card->log_bus_width));
		else if (!card->is_sdio) // make sure card is SD
			fprintf(stream, "SSR: bus_width=%d\n", (card->ssr.cur_bus_width ? 4 : 1));
	}
	if (print_scr)
		fprintf(stream, "SCR: sd_spec=%d, bus_width=%d\n", card->scr.sd_spec, card->scr.bus_width);

	size_t file_size = MIN(buf_size, ftell(stream));

	fseek(stream, 0, SEEK_SET);

	for (int i = 0; i < file_size - 1; ++i)
		buf[i] = fgetc(stream);

	fclose(stream);

	if (remove(TMP_FILE_TXT) != 0)
		printf(CANT_REMOVE_FILE_TMPLT, TAG, TMP_FILE_TXT);

	return ESP_OK;
}

esp_err_t sd_spi_init(void)
{
	esp_err_t ret;

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
			.format_if_mount_failed = true,
			.max_files = 10,
			.allocation_unit_size = 32 * 1024
	};
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");
	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.do_transaction = wrap_sdspi_host_do_transaction;

	spi_bus_config_t bus_cfg = {
			.mosi_io_num = SPI_NUM_MOSI,
			.miso_io_num = SPI_NUM_MISO,
			.sclk_io_num = SPI_NUM_SCK,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.max_transfer_sz = 4000,
	};

	ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
	if (ret != ESP_OK)
	{
		glob_set_bits_status_err(STATUS_ERROR_SD);
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return ret;
	}

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = SPI_NUM_CS;
	slot_config.host_id = host.slot;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK)
	{
		glob_set_bits_status_err(STATUS_ERROR_SD);
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount filesystem. "
					"If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
					"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return ret;
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	return ret;
}

void sd_spi_space_info(uint32_t *totalKB, uint32_t *availableKB)
{
	if (totalKB == NULL || availableKB == NULL)
		return;

	FATFS *fs;
	uint32_t fre_clust, fre_sect, tot_sect;

	/* Get volume information and free clusters of drive 0 */
	f_getfree("0:", &fre_clust, &fs);
	/* Get total sectors and free sectors */
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	fre_sect = fre_clust * fs->csize;

	/* Print the free space (assuming 512 bytes/sector) */
//	printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2, fre_sect / 2);

	*totalKB = tot_sect / 2;
	*availableKB = fre_sect / 2;
}

sdmmc_card_t *sd_spi_card()
{
	return card;
}
//void sd_spi_deinit(void)
//{
//	const char mount_point[] = MOUNT_POINT;
//
//	// All done, unmount partition and disable SPI peripheral
//	esp_vfs_fat_sdcard_unmount(mount_point, card);
//	ESP_LOGI(TAG, "Card unmounted");
//
//	//deinitialize the bus after all devices are removed
//	spi_bus_free(host.slot);
//}
