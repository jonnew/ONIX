#include "config.h"
#include "main_menu.hpp"

static void list_create(void);
static void list_focus_cb(lv_group_t * group);
static void group_style_mod_cb(lv_group_t * group, lv_style_t * style);

static lv_obj_t * list; // This is the menu
static lv_group_t * g; // The is the group for the inteactiable content

// IO handler
// TODO: each button should have its own event handler.
// These can then create sub menus with structures that look very similar to
// this file
static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        // Nothing
        //printf("Clicked: %s\n", lv_list_get_btn_text(obj));
    }
}

// Main menu is aligned to the bottom of the header
void main_menu_create(lv_obj_t * header, lv_indev_t * dev)
{
    // Create control group,
    g = lv_group_create();

    // Assign sytle modification callback to make it look pretty, and assign hardware
    lv_group_set_style_mod_cb(g, group_style_mod_cb);

    // Create the main menu (list of buttons for each major function)
    // Assign each button to the main menu group (g)
    lv_coord_t hres = lv_disp_get_hor_res(NULL);
    lv_coord_t vres = lv_disp_get_ver_res(NULL);

    list = lv_list_create(lv_disp_get_scr_act(NULL), NULL);
    lv_list_set_style(list, LV_PAGE_STYLE_BG, &lv_style_plain);
    lv_obj_set_size(list, hres, vres - lv_obj_get_height(header));
    lv_obj_align(list, header, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_list_set_edge_flash(list, true);

    lv_obj_t * list_btn;

    list_btn = lv_list_add_btn(list, LV_SYMBOL_SHUFFLE, "Remote Links");
    lv_obj_set_event_cb(list_btn, event_handler);
    lv_group_add_obj(g, list_btn);
    lv_obj_set_protect(list_btn, LV_PROTECT_CLICK_FOCUS);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Analog GPIO");
    lv_obj_set_event_cb(list_btn, event_handler);
    lv_group_add_obj(g, list_btn);
    lv_obj_set_protect(list_btn, LV_PROTECT_CLICK_FOCUS);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Digital GPIO");
    lv_obj_set_event_cb(list_btn, event_handler);
    lv_group_add_obj(g, list_btn);
    lv_obj_set_protect(list_btn, LV_PROTECT_CLICK_FOCUS);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_AUDIO, "Audio");
    lv_obj_set_event_cb(list_btn, event_handler);
    lv_group_add_obj(g, list_btn);
    lv_obj_set_protect(list_btn, LV_PROTECT_CLICK_FOCUS);

    // Set group focus callback
    lv_group_set_focus_cb(g, list_focus_cb);

    // Assign hardware input device to control the group
    lv_indev_set_group(dev, g);
}

// Scroll the list to show selected button
static void list_focus_cb(lv_group_t * group)
{
    lv_obj_t * f = lv_group_get_focused(group);
    lv_list_focus(f, LV_ANIM_ON);
}

// Make the selected style a yellow outline
static void group_style_mod_cb(lv_group_t * group, lv_style_t * style)
{
    (void *) group; // not used

    // Make fill colors a bit yellow
    style->body.main_color = lv_color_mix(style->body.main_color, LV_COLOR_YELLOW, LV_OPA_50);
    style->body.grad_color = lv_color_mix(style->body.grad_color, LV_COLOR_YELLOW, LV_OPA_50);

    // Blend the text color
    //style->text.color = lv_color_mix(style->text.color, LV_COLOR_YELLOW, LV_OPA_50);
}
