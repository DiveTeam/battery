#ifndef WAKEUP_H
#define WAKEUP_H

#include "Arduino.h"

// RTC_DATA_ATTR int currentStatus = 0;

void RTC_IRAM_ATTR wake_stub();
void print_wakeup_reason();

#endif
