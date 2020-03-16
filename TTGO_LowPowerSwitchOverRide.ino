/*
  Low Power Mode entered at start up (GPS, LORA and USB off)
  Calls MyInterruptRoutine(); after each wake up event...
  To reprogram teh board....
  Push and hold the SENSOR_VN until the TTGO wakes up from the timer interrupt to enter the
  stardard loop() routine and NEVER enter sleep - this is required to upload firmware to the TTGO
  Modifications undertaken by Adrian Keating AT The University of Westerna Australia
  March 2020
*/
#include <string.h>
#include <ctype.h>
#include <WiFi.h>
#include<esp_deep_sleep.h>

#define BUILTIN_LED 14
#define ME6211_EN 21
#define SET_FLASHON true   // Use this flag if you are unsure the TTGO is responding during wake periods
#define SET_RTC_OFF true
#define SET_WIFI_OFF true
#define SET_BLUETOOTH_OFF true
#define SENSOR_VN 39

// stores the data on the RTC memory so that it will not be deleted during the deep sleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int ResetCounter = 0;   // count loops
boolean ULPower_flag = false;

// deep sleep
#define uS_TO_S_FACTOR 1000000  // conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  10        // time ESP32 will go to sleep (in seconds)   - 99 for (about) 1% duty cycle  

void setup() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  // Reading switch state
  if (analogRead(SENSOR_VN) < 2048)  {
    pinMode(BUILTIN_LED, OUTPUT);
    ULPower_flag = false; //  Switch has been pushed during power up, so NEVER go to sleep
    GPS_LoRA_USB_On();
    MyBlink(5); // start a slow blink with 0.5 sec delays
  } else {
    GPS_LoRA_USB_Off();   // RUN as soon as possible AFTER setup starts
    ULPower_flag = true;
    pinMode(BUILTIN_LED, OUTPUT);
  }

  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)  MyInterruptRoutine();

  if (ULPower_flag) {
    digitalWrite(BUILTIN_LED, LOW);
    ResetCounter++;
    if (SET_WIFI_OFF) {
      WiFi.mode(WIFI_OFF);
      delay(50);
    }
    if (SET_BLUETOOTH_OFF) {
      btStop();
      delay(50);
    }

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    if (SET_RTC_OFF) {
      esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_FAST_MEM=2
      esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_SLOW_MEM=1
      esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);  //ESP_PD_DOMAIN_RTC_PERIPH=0
      esp_deep_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
      esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
    }
    esp_deep_sleep_start();   // enters deep sleep with the configured wakeup options
  }
}

void loop() {
  // if SENSOR_VN pushed, program will loop here until reset or reprogrammed and NEVER enter sleep mode
}

void GPS_LoRA_USB_On() {
  pinMode(ME6211_EN, OUTPUT);
  digitalWrite(ME6211_EN, HIGH);  // Enable power to GPS, LoRA and USB controller
}

void GPS_LoRA_USB_Off() {
  pinMode(ME6211_EN, OUTPUT);
  digitalWrite(ME6211_EN, LOW);  // Enable power to GPS, LoRA and USB controller
}

void MyInterruptRoutine() {
  //BLINK JUST FOR SHOW
  MyBlink(1);  //Fast blink
}

void MyBlink(int rate) {
  pinMode(BUILTIN_LED, OUTPUT);
  for (int ii = 0; ii < 10; ii++) {
    // if SET_FLASHON = false no flash occurs but delay still occurs
    if (SET_FLASHON) digitalWrite(BUILTIN_LED, HIGH);
    delay(rate * 100);
    if (SET_FLASHON) digitalWrite(BUILTIN_LED, LOW);
    delay(rate * 100);
  }
}


