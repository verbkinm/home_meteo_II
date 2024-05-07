/*
 * sd.c
 *
 *  Created on: 7 мая 2024 г.
 *      Author: user
 */

#include "sd.h"

extern lv_obj_t *sub_sd_page;

static const char *TAG = "SETTING SD";

static int sd_info(char *buf, size_t buf_size);

static int sd_info(char *buf, size_t buf_size)
{
	if (buf == NULL)
		return -1;

	sdmmc_card_t *card = sd_spi_card();

	FILE *stream = fopen(TMP_FILE_TXT, "w+");
	if (stream == NULL)
	{
		printf(CANT_WRITE_FILE_TMPLT, TAG, TMP_FILE_TXT);
		return -1;
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

	//    fprintf(stream, "Размер: %lluMB\n", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));

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
		fprintf(stream, "SCR: sd_spec=%d, bus_width=%d", card->scr.sd_spec, card->scr.bus_width);

	size_t file_size = MAX(buf_size, ftell(stream));

	fseek(stream, 0, SEEK_SET);

	for (int i = 0; i < file_size - 1; ++i)
		buf[i] = fgetc(stream);

	fclose(stream);
	if (remove(TMP_FILE_TXT) != 0)
		printf(CANT_REMOVE_FILE_TMPLT, TAG, TMP_FILE_TXT);

	return 0;
}

void create_sd_sub_page(lv_event_t *e)
{
	clear_all_sub_page_child();

	lv_obj_set_style_pad_hor(sub_sd_page, 20, 0);
	lv_obj_t *section = lv_menu_section_create(sub_sd_page);


	char *buf = calloc(1, BUFSIZE);

	if (sd_info(buf, BUFSIZE) == 0)
		create_text(section, NULL, buf, LV_MENU_ITEM_BUILDER_VAR_1);

	free(buf);
}

void free_sd_sub_page(void)
{

}
