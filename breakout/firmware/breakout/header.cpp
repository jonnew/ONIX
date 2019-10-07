#include "config.h"
#include "header.hpp"

// Header
lv_obj_t * header_create(lv_obj_t * scr)
{
    lv_obj_t * header;

    lv_coord_t vres = lv_disp_get_ver_res(NULL);

    header = lv_cont_create(scr, NULL); // lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_width(header, lv_disp_get_hor_res(NULL));
    lv_obj_set_height(header, (vres * HDR_HEIGHT_PER)/100);

    // Header text style
    static lv_style_t hdr_style; // Styles can't be local variables
    lv_style_copy(&hdr_style, &lv_style_plain); // Copy a built-in style as a starting point
    hdr_style.text.font	= &lv_font_roboto_12;
    hdr_style.body.main_color = LV_COLOR_SILVER;
    hdr_style.body.grad_color = LV_COLOR_SILVER;

	lv_obj_set_style(header, &hdr_style);

    lv_obj_t * sym = lv_label_create(header, NULL);
    lv_label_set_text(sym, LV_SYMBOL_MUTE);
    lv_obj_align(sym, NULL, LV_ALIGN_IN_RIGHT_MID, -LV_DPI/10, 0);

    lv_obj_t * clock = lv_label_create(header, NULL);
    lv_label_set_text(clock, "000:00:00.0");
    lv_obj_align(clock, NULL, LV_ALIGN_IN_LEFT_MID, LV_DPI/10, 0);

    lv_obj_set_pos(header, 0, 0);

    return header;
}
