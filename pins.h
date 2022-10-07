#ifndef _PINS_H_
#define _PINS_H_

#define PIN_TIME_MEASURE    12                  // Core1 PDM処理時間計測用
#define PIN_PIOT_MEASURE    13                  // Core1 PIO処理時間計測用
#define PIN_USBINT_PERIOD   18                  // Core0 USB処理時間計測用
#define PIN_OUTPUT_RP       14                  // LCh P
#define PIN_OUTPUT_RN       15                  // LCh N
#define PIN_OUTPUT_LP       16                  // RCh P
#define PIN_OUTPUT_LN       17                  // RCh N
#define PIN_DCDC_PWM        23                  // DCDC PFM_PWM Control pin
#define PIN_DIP_0           10                   // Test DipSW start pin
#define PIN_DIP_MASK        (0xFF << PIN_DIP_0) // Test DipSW Mask position
#define PIN_DIP_BIT_WIDTH   8                   // Test DipSW Bit width

#endif
