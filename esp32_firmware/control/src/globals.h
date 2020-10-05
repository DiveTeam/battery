#ifndef GLOBALS_H
#define GLOBALS_H

#include "max1720x.h"


static max1720x gauge;

struct log_record
{
    uint32_t record_id;
    uint8_t soc;
    uint8_t is_leaking;
    float temperature;
    float current;
    float v1;
    float v2;
    float v3;
};

typedef struct log_record log_record;

extern volatile uint8_t wanna_turn_off;
extern volatile unsigned long millis_since_start;
extern volatile unsigned long last_led_processed_at;
extern volatile unsigned long last_load_detected_at;
extern volatile unsigned long last_record_saved_at;
extern volatile unsigned long last_ble_notified_at;
extern volatile unsigned long last_gauge_updated_at;
extern volatile unsigned long last_gauge_updatec_characteristics;


extern volatile uint16_t leakage_blinks_made;
extern log_record current_state;
extern int64_t intention_to_read_address;

#endif
