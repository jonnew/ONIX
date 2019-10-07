#include "ssd1351.hpp"

// NB: If there are performance issues, optimized SPI communication procedures can be found
// here: https://github.com/PaulStoffregen/ILI9341_t3/blob/8437e539e44256b39c42d6f0e5e42af7ac42676f/ILI9341_t3.h#L334
void ssd1351_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}
