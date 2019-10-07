// Scratch pad for display developement

#include <lvgl.h>
#include <Adafruit_SSD1351.h>
#include <Encoder.h>

//#include "logo.h"
#include "rotary_enc.hpp"
#include "ssd1351.hpp"
#include "header.hpp"
#include "main_menu.hpp"

// OLED dimensions
#define SCREEN_PIX 128

// OLED pins
#define SCLK_PIN 13
#define MOSI_PIN 11
#define DC_PIN   8
#define CS_PIN   10
#define RST_PIN  14

// Rotary encoder input pins
#define ENC_A 15
#define ENC_B 16

#define USE_LV_LOG 1
#define LV_UPDATE_MSEC 5

// T4 bug?
// Needed to use some parts of modern c++ (e.g. std::queue and std:vector)
unsigned __exidx_start;
unsigned __exidx_end;

// LVGL stuff
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

// Use hardware SPI
Adafruit_SSD1351 tft(SCREEN_PIX, SCREEN_PIX, &SPI, CS_PIN, DC_PIN, RST_PIN);
//Logo<7> logo(0.1, 128, 0, 0, 10);

// Rotary encoder
Encoder encoder(ENC_A, ENC_B);
// TODO: timer that checks state of all input devices and calls there *_handler functions with the result

// lv display update task
IntervalTimer lv_timer;

void lv_update_isr(void) { lv_tick_inc(LV_UPDATE_MSEC); }

static uint16_t last_position = 0;

// For some reason, cannot link when in rotary.xpp
bool enc_update(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void) indev;      /*Unused*/

    uint16_t position = encoder.read();

    /*Save the state and save the pressed coordinate*/
    //data->state = encoder_is_pressed() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->state =  LV_INDEV_STATE_REL;

    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->enc_diff = position - last_position ;
    Serial.println(data->enc_diff);
    last_position = position;

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

void setup(void) {

    // Start up lvgl
    lv_init();

    // Setup the display
    tft.begin();

    // Initialize the display buffer
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    // Initialize the display
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = ssd1351_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize and register the rotary encoder
    lv_indev_drv_t encoder_drv;
    lv_indev_drv_init(&encoder_drv);
    encoder_drv.type = LV_INDEV_TYPE_ENCODER;
    encoder_drv.read_cb = enc_update;
    lv_indev_t * enc_indev = lv_indev_drv_register(&encoder_drv);

    // Set theme and create base object
    lv_theme_t *thm = lv_theme_mono_init(0, NULL);
    lv_theme_set_current(thm);
    lv_obj_t * scr = lv_obj_create(NULL, NULL);
    lv_disp_load_scr(scr);

    // Header (passive, reacts to changes in acqusition state)
    lv_obj_t * header = header_create(scr);

    // Main menu (active, hierarchy of controls mutates acqusition state)
    main_menu_create(header, enc_indev); 

    Serial.begin(9600);
    //lv_log_register_print(my_print); /* register print function for debugging */

    // Screen update routine
    lv_timer.begin(lv_update_isr, LV_UPDATE_MSEC * 1000);
}

uint8_t i = 0;
void loop(void)
{

    lv_task_handler(); /* let the GUI do its work */
    delay(5);
    //logo.draw(tft);
}
