idf_component_register(SRCS "main.c"
							"TFT_touchscreen/TFT_touch_screen.c"
							"GUI_manager/GUI_manager.c"
							"status_panel/status_panel.c"
							"I2C/BME280.c"
							"I2C/i2c.c"
							"I2C/DS3231"
							"Local_Lib/local_lib.c"
							"Local_Lib/local_lvgl_lib.c"
							"Local_Lib/widget/bar.c"
							"sd/sd_spi.c"
							"screen_page/startscreen/startscreen.c"
							"screen_page/page.c"
							"screen_page/screendefault.c"
							"screen_page/homepage/homepage.c"
							"screen_page/homepage/block/block.c"
							"screen_page/homepage/block/weather/weather.c"
							"screen_page/datetime1/datetime1.c"
							"screen_page/datetime2/datetime2.c"
							"screen_page/datetime3/datetime3.c"
							"screen_page/meteo_chart/meteo_chart.c"
							"screen_page/menupage.c"
							"screen_page/full_screen_page/full_screen_page.c"
							"screen_page/setting/main_page/main_page.c"
							"screen_page/setting/settingpage.c"
							"screen_page/setting/elements.c"
							"screen_page/setting/wifi/wifi.c"
							"screen_page/setting/date_time/date_time.c"
							"screen_page/setting/weather/weather.c"
							"screen_page/setting/display/display.c"
							"screen_page/setting/update/update.c"
							"screen_page/setting/sensors/sensors.c"
							"screen_page/setting/remote_sensors/remote_sensors.c"
							"screen_page/setting/sd/sd.c"
							"screen_page/home_meteo/home_meteo.c"
							"json/json_config.c"
							"service/service.c"
							"service/wifi/wifi.c"
							"service/sntp/sntp.c"
							"service/weather/weather.c"
							"service/display/display.c"
							"service/bme280/bme280.c"
							"service/update/update.c"
							"service/udp_server/udp_server.c"
							"service/tcp_client/tcp_client.c"
							"fonts/ubuntu_mono_14.c"
							"fonts/ubuntu_mono_32.c"
							"fonts/ubuntu_mono_26.c"
							"fonts/ubuntu_mono_48.c"
							"fonts/ubuntu_mono_64.c"
							"fonts/ubuntu_mono_128.c"
							"fonts/ubuntu_mono_148.c"
							"img_c/sd.c"
							"ota/ota.c" 
							"Protocols/embedded/creatorpkgs.c"
							"Protocols/embedded/header.c"
							"Protocols/embedded/host_broadcast.c"
							"Protocols/embedded/identification.c"
							"Protocols/embedded/iotv_server_embedded.c"
							"Protocols/embedded/IOTV_SH.c"
							"Protocols/embedded/log_data.c"
							"Protocols/embedded/read_write.c"
							"Protocols/embedded/state.c"
							"Protocols/embedded/tech.c"
							"iotv.c"
                       INCLUDE_DIRS ".")
