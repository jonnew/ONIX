#pragma once

#include <Encoder.h>
#include <lvgl.h>

//extern Button enc_butt;
extern Encoder encoder;

// Get encoder (i.e. mouse wheel) ticks difference and pressed state
// @param indev_drv pointer to the related input device driver
// @param data store the read data here
// @return false: all ticks and button state are handled
bool encoder_update(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

// It is called periodically from the SDL thread to check a key is pressed/released
// @param event describes the event
//void rotary_handler(rotary_event_t *event);
