#pragma once

#include <lvgl.h>
#include <Adafruit_SSD1351.h>

extern Adafruit_SSD1351 tft;
void ssd1351_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
