// #include "globals.h"
#include "wakeup.h"
#include "Arduino.h"

//headers to handle deep sleep wakeup registers properly
#include "rom/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/timer_group_reg.h"


#define BTN_GPIO_NUM 34
#define BTN_RTC_GPIO_NUM 4

#define CHARGER_GPIO_NUM 35
#define CHARGER_RTC_GPIO_NUM 5

#define BTN_IS_LOW() \
    ((REG_GET_FIELD(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT) \
            & BIT(BTN_RTC_GPIO_NUM)) == 0)

#define CHARGER_IS_LOW() \
    ((REG_GET_FIELD(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT) \
            & BIT(CHARGER_RTC_GPIO_NUM)) == 0)


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by button"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by charger"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


void RTC_IRAM_ATTR wake_stub(){
    if(!BTN_IS_LOW()){ //button is not low, but we are waking up. charger is on?
      if(CHARGER_IS_LOW()){
        // On revision 0 of ESP32, this function must be called:
          esp_default_wake_deep_sleep();
          return; //continue booting
      }
    }else{//button pressed. will debounce and continue
      uint16_t ticks = 0;
      do {
          if(BTN_IS_LOW()){
            ticks++;
          }
          // wait 1ms
          ets_delay_us(1000);
      } while (BTN_IS_LOW());

      if(ticks > 20){ //>20ms = debounced, waking up
          // On revision 0 of ESP32, this function must be called:
          esp_default_wake_deep_sleep();
          return; //continue booting
      }
    }
    
    
    //sporadic click, sleeping firther

    // Set the pointer of the wake stub function.
    REG_WRITE(RTC_ENTRY_ADDR_REG, (uint32_t)&wake_stub);
    // Go to sleep.
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    // A few CPU cycles may be necessary for the sleep to start...
    while (true) {
        ;
    }
}
