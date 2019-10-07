#include "rotary_enc.hpp"

static uint16_t last_position = 0;
//static int16_t enc_diff = 0;
//static lv_indev_state_t state = LV_INDEV_STATE_REL;

static bool encoder_is_pressed()
{
    return false;
}

bool encoder_update(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void) indev;

    uint16_t position = encoder.read();

    /*Save the state and save the pressed coordinate*/
    data->state = encoder_is_pressed() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->enc_diff = position - last_position ;
    last_position = position;

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

/**
 * It is called periodically from the SDL thread to check mouse wheel state
 * @param event describes the event
 */
//void rotary_handler(rotary_event_t *event)
//{
//    // TODO: actual encorder reading stuff
//    switch(event->type) {
//        case ENC_RIGHT:
//            state = LV_KEY_RIGHT;
//            break;
//
//        case ENC_LEFT:
//            state = LV_KEY_LEFT;
//            break;
//            enc_diff = -event->wheel.y;
//
//        case ENC_BUTTONDOWN:
//            state = LV_KEY_ENTER;
//            break;
//
//        default:
//            break;
//    }
//}
