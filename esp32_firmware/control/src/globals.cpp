#include "globals.h"

volatile uint8_t wanna_turn_off = 0;
volatile unsigned long millis_since_start;
volatile unsigned long last_led_processed_at = 0;
volatile unsigned long last_load_detected_at = 0;
volatile unsigned long last_record_saved_at = 0;
volatile unsigned long last_gauge_updated_at = 0;
volatile unsigned long last_gauge_updatec_characteristics = 0;
volatile unsigned long last_ble_notified_at = 0;
volatile uint16_t leakage_blinks_made = 0;
log_record current_state;
int64_t intention_to_read_address = -1;

